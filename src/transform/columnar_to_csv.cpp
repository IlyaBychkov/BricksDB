#include "transform/columnar_to_csv.h"

#include <expected>
#include <string>

#include "schema/batch.h"
#include "schema/schema.h"
#include "transform/metadata.h"

ColumnarToCsvTransformer::ColumnarToCsvTransformer(const std::string& columnar_filename,
                                                   const std::string& schema_filename,
                                                   const std::string& csv_filename)
    : columnar_filename_(columnar_filename),
      schema_filename_(schema_filename),
      csv_filename_(csv_filename) {
}

ColumnarToCsvTransformer::~ColumnarToCsvTransformer() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

std::expected<void, std::string> ColumnarToCsvTransformer::Prepare() {
    fin_.open(columnar_filename_, std::ios::binary);
    if (!fin_.is_open()) {
        return std::unexpected(
            std::string("ColumnarToCsvTransformer::Prepare: Failed to open columnar file '") +
            columnar_filename_ + "'");
    }

    auto tmp_csv = CreateCsvWriter(csv_filename_);
    if (!tmp_csv) {
        return std::unexpected(
            std::string("ColumnarToCsvTransformer::Prepare: CreateCsvWriter failed for '") +
            csv_filename_ + "': " + tmp_csv.error());
    }
    csv_out_ = std::move(*tmp_csv);

    return {};
}

std::expected<void, std::string> ColumnarToCsvTransformer::Transform() {
    auto prepare_res = Prepare();
    if (!prepare_res) {
        return std::unexpected("Prepare failed: " + prepare_res.error());
    }

    auto metadata_res = ReadMetadataFromFile(fin_);
    if (!metadata_res) {
        return std::unexpected(
            std::string("ColumnarToCsvTransformer::Transform: ReadMetadataFromFile failed for '") +
            columnar_filename_ + "': " + metadata_res.error());
    }
    Metadata metadata = *metadata_res;

    Schema schema = metadata.GetSchema();
    WriteSchemaToCsv(schema, schema_filename_);

    std::vector<int64_t>& offsets = metadata.GetOffsets();
    std::vector<int64_t>& rows = metadata.GetRowsCnt();

    for (size_t i = 0; i < offsets.size(); ++i) {
        fin_.seekg(offsets[i], std::ios::beg);
        auto batch_tmp = CreateBatchFromFile(schema, fin_, rows[i]);
        if (!batch_tmp) {
            return std::unexpected(
                std::string(
                    "ColumnarToCsvTransformer::Transform: CreateBatchFromFile failed at offset ") +
                std::to_string(offsets[i]) + ": " + batch_tmp.error());
        }
        Batch batch = std::move(*batch_tmp);
        auto res = csv_out_.WriteBatch(batch);
        if (!res) {
            return std::unexpected(
                std::string("ColumnarToCsvTransformer::Transform: WriteBatchToCsv failed while "
                            "processing batch at offset ") +
                std::to_string(offsets[i]) + ": " + res.error());
        }
    }

    if (csv_out_.IsCrashed()) {
        return std::unexpected("CsvWriter crashed");
    }

    return {};
}
