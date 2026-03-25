#include "transform/columnar_to_csv.h"

#include <chrono>
#include <expected>
#include <format>
#include <string>

#include "scheme/batch.h"
#include "scheme/column.h"
#include "scheme/scheme.h"
#include "scheme/type.h"
#include "transform/metadata.h"

ColumnarToCSVTransformer::ColumnarToCSVTransformer(const std::string& columnar_filename,
                                                   const std::string& scheme_filename,
                                                   const std::string& csv_filename)
    : columnar_filename_(columnar_filename),
      scheme_filename_(scheme_filename),
      csv_filename_(csv_filename) {
}

ColumnarToCSVTransformer::~ColumnarToCSVTransformer() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

std::expected<void, std::string> ColumnarToCSVTransformer::Prepare() {
    fin_.open(columnar_filename_, std::ios::binary);
    if (!fin_.is_open()) {
        return std::unexpected(
            std::string("ColumnarToCSVTransformer::Prepare: Failed to open columnar file '") +
            columnar_filename_ + "'");
    }

    auto tmp_csv = CreateCSVWriter(csv_filename_);
    if (!tmp_csv) {
        return std::unexpected(
            std::string("ColumnarToCSVTransformer::Prepare: CreateCSVWriter failed for '") +
            csv_filename_ + "': " + tmp_csv.error());
    }
    csv_out_ = std::move(*tmp_csv);

    return {};
}

std::expected<void, std::string> ColumnarToCSVTransformer::Transform() {
    auto prepare_res = Prepare();
    if (!prepare_res) {
        return std::unexpected("Prepare failed: " + prepare_res.error());
    }

    auto metadata_res = ReadMetadataFromFile(fin_);
    if (!metadata_res) {
        return std::unexpected(
            std::string("ColumnarToCSVTransformer::Transform: ReadMetadataFromFile failed for '") +
            columnar_filename_ + "': " + metadata_res.error());
    }
    Metadata metadata = *metadata_res;

    Scheme scheme = metadata.GetScheme();
    WriteSchemeToFile(scheme, scheme_filename_);

    std::vector<int64_t>& offsets = metadata.GetOffsets();
    std::vector<int64_t>& rows = metadata.GetRowsCnt();

    for (size_t i = 0; i < offsets.size(); ++i) {
        fin_.seekg(offsets[i], std::ios::beg);
        auto batch_tmp = CreateBatchFromFile(scheme, fin_, rows[i]);
        if (!batch_tmp) {
            return std::unexpected(
                std::string(
                    "ColumnarToCSVTransformer::Transform: CreateBatchFromFile failed at offset ") +
                std::to_string(offsets[i]) + ": " + batch_tmp.error());
        }
        Batch batch = std::move(*batch_tmp);
        auto res = WriteBatchToCSV(batch);
        if (!res) {
            return std::unexpected(
                std::string("ColumnarToCSVTransformer::Transform: WriteBatchToCSV failed while "
                            "processing batch at offset ") +
                std::to_string(offsets[i]) + ": " + res.error());
        }
    }

    if (csv_out_.IsCrashed()) {
        return std::unexpected("CSVWriter crashed");
    }

    return {};
}

std::expected<void, std::string> ColumnarToCSVTransformer::WriteBatchToCSV(const Batch& batch) {
    for (size_t i = 0; i < batch.RowsCnt(); ++i) {
        std::vector<std::string> row;
        for (size_t c = 0; c < batch.ColumnsCnt(); ++c) {
            Type t = batch.GetColumnType(c);
            if (t == Type::int64) {
                const auto& val = batch.GetColumn(c).GetValue<int64_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::int32) {
                const auto& val = batch.GetColumn(c).GetValue<int32_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::int16) {
                const auto& val = batch.GetColumn(c).GetValue<int16_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::string) {
                const auto& val = batch.GetColumn(c).GetValue<std::string>(i);
                row.push_back(val);
            } else if (t == Type::timestamp) {
                const auto& val = batch.GetColumn(c).GetValue<int64_t>(i);
                std::chrono::sys_seconds ts{std::chrono::seconds{val}};
                row.push_back(std::format("{:%F %T}", ts));
            } else if (t == Type::date) {
                const auto& val = batch.GetColumn(c).GetValue<int32_t>(i);
                std::chrono::sys_days dt{std::chrono::days{val}};
                row.push_back(std::format("{:%F}", dt));
            } else {
                return std::unexpected(std::string("ColumnarToCSVTransformer::WriteBatchToCSV: "
                                                   "Unsupported column type at column ") +
                                       std::to_string(c));
            }
        }
        auto res = csv_out_.WriteRow(row);
        if (!res) {
            return std::unexpected(
                std::string(
                    "ColumnarToCSVTransformer::WriteBatchToCSV: CSVWriter WriteRow failed: ") +
                res.error());
        }
    }
    if (csv_out_.IsCrashed()) {
        return std::unexpected(
            std::string(
                "ColumnarToCSVTransformer::WriteBatchToCSV: CSVWriter crashed while writing to '") +
            csv_filename_ + "'");
    }
    return {};
}