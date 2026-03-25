#include "transform/csv_to_columnar.h"

#include "transform/metadata.h"

CSVToColumnarTransformer::CSVToColumnarTransformer(const std::string& csv_filename,
                                                   const std::string& scheme_filename,
                                                   const std::string& columnar_filename,
                                                   int64_t batch_max_size)
    : csv_filename_(csv_filename),
      scheme_filename_(scheme_filename),
      batch_max_size_(batch_max_size),
      columnar_filename_(columnar_filename) {
}

CSVToColumnarTransformer::~CSVToColumnarTransformer() {
    if (fout_.is_open()) {
        fout_.close();
    }
}

std::expected<void, std::string> CSVToColumnarTransformer::Prepare() {
    auto tmp = CreateCSVBatcher(csv_filename_, scheme_filename_, batch_max_size_);
    if (!tmp) {
        return std::unexpected(
            std::string("CSVToColumnarTransformer::Prepare: CreateCSVBatcher failed for csv='") +
            csv_filename_ + "' scheme='" + scheme_filename_ + "': " + tmp.error());
    }
    batcher_ = std::move(tmp.value());
    fout_.open(columnar_filename_, std::ios::binary);
    if (!fout_.is_open()) {
        return std::unexpected(
            std::string("CSVToColumnarTransformer::Prepare: Failed to open columnar file '") +
            columnar_filename_ + "'");
    }
    return {};
}

std::expected<void, std::string> CSVToColumnarTransformer::Transform() {
    auto prepare_res = Prepare();
    if (!prepare_res) {
        return std::unexpected("Prepare failed: " + prepare_res.error());
    }

    Metadata metadata(batcher_.GetScheme());

    while (batcher_.HasNextBatch()) {
        auto batch_tmp = batcher_.NextBatch();
        if (!batch_tmp) {
            return std::unexpected(
                std::string("CSVToColumnarTransformer::Transform: CSVBatcher NextBatch failed: ") +
                batch_tmp.error());
        }
        Batch batch = std::move(*batch_tmp);
        metadata.AddRowGroup(fout_.tellp(), batch.RowsCnt());
        auto res = WriteBatch(batch);
        if (!res) {
            return std::unexpected(std::string("CSVToColumnarTransformer::Transform: WriteBatch "
                                               "failed while processing batch at offset ") +
                                   std::to_string(static_cast<long long>(fout_.tellp())) + ": " +
                                   res.error());
        }
    }

    auto res = WriteMetadataToFile(metadata, fout_);
    if (!res) {
        return std::unexpected(
            std::string("CSVToColumnarTransformer::Transform: WriteMetadataToFile failed for '") +
            columnar_filename_ + "': " + res.error());
    }

    if (batcher_.IsCrashed()) {
        return std::unexpected(std::string("CSVToColumnarTransformer::Transform: Batcher crashed "
                                           "during transformation for csv='") +
                               csv_filename_ + "'");
    }
    return {};
}

std::expected<void, std::string> CSVToColumnarTransformer::WriteBatch(const Batch& batch) {
    for (const auto& column : batch.GetAllColumns()) {
        auto res = WriteColumnToColumnar(column, fout_);
        if (!res) {
            return std::unexpected(res.error());
        }
    }

    if (!fout_.good()) {
        return std::unexpected("Failed to write batch to columnar file");
    }
    return {};
}