#include "transform/csv_to_columnar.h"

#include <iostream>

#include "transform/metadata.h"

CsvToColumnarTransformer::CsvToColumnarTransformer(const std::string& csv_filename,
                                                   const std::string& schema_filename,
                                                   const std::string& columnar_filename,
                                                   int64_t max_batch_size_bytes)
    : csv_filename_(csv_filename),
      schema_filename_(schema_filename),
      max_batch_size_bytes_(max_batch_size_bytes),
      columnar_filename_(columnar_filename) {
}

CsvToColumnarTransformer::~CsvToColumnarTransformer() {
    if (fout_.is_open()) {
        fout_.close();
    }
}

std::expected<void, std::string> CsvToColumnarTransformer::Prepare() {
    auto tmp = CreateCsvBatcher(csv_filename_, schema_filename_, max_batch_size_bytes_);
    if (!tmp) {
        return std::unexpected(
            std::string("CsvToColumnarTransformer::Prepare: CreateCsvBatcher failed for csv='") +
            csv_filename_ + "' schema='" + schema_filename_ + "': " + tmp.error());
    }
    batcher_ = std::move(tmp.value());
    fout_.open(columnar_filename_, std::ios::binary);
    if (!fout_.is_open()) {
        return std::unexpected(
            std::string("CsvToColumnarTransformer::Prepare: Failed to open columnar file '") +
            columnar_filename_ + "'");
    }
    return {};
}

std::expected<void, std::string> CsvToColumnarTransformer::Transform() {
    auto prepare_res = Prepare();
    if (!prepare_res) {
        return std::unexpected("Prepare failed: " + prepare_res.error());
    }

    Metadata metadata(batcher_.GetSchema());

    while (batcher_.HasNextBatch()) {
        auto batch_tmp = batcher_.NextBatch();
        if (!batch_tmp) {
            return std::unexpected(
                std::string("CsvToColumnarTransformer::Transform: CsvBatcher NextBatch failed: ") +
                batch_tmp.error());
        }
        Batch batch = std::move(*batch_tmp);
        metadata.AddRowGroup(fout_.tellp(), batch.RowsCnt());
        auto res = WriteBatch(batch);
        if (!res) {
            return std::unexpected(std::string("CsvToColumnarTransformer::Transform: WriteBatch "
                                               "failed while processing batch at offset ") +
                                   std::to_string(static_cast<long long>(fout_.tellp())) + ": " +
                                   res.error());
        }
    }

    auto res = WriteMetadataToFile(metadata, fout_);
    if (!res) {
        return std::unexpected(
            std::string("CsvToColumnarTransformer::Transform: WriteMetadataToFile failed for '") +
            columnar_filename_ + "': " + res.error());
    }

    if (batcher_.IsCrashed()) {
        return std::unexpected(std::string("CsvToColumnarTransformer::Transform: Batcher crashed "
                                           "during transformation for csv='") +
                               csv_filename_ + "'");
    }
    return {};
}

std::expected<void, std::string> CsvToColumnarTransformer::WriteBatch(const Batch& batch) {
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
