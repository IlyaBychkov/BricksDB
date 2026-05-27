#include "operators/scan_operator.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "schema/batch.h"
#include "schema/type.h"
#include "transform/metadata.h"

ScanOperator::ScanOperator(const std::string& filename, const std::set<std::string>& columns)
    : columns_(columns) {
    fin_.open(filename, std::ios::binary);
    if (!fin_.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    auto metadata_res = ReadMetadataFromFile(fin_);
    if (!metadata_res) {
        throw std::runtime_error("Failed to read metadata from file '" + filename +
                                 "': " + metadata_res.error());
    }
    metadata_ = *metadata_res;
    fin_.seekg(0, std::ios::beg);
}

ScanOperator::~ScanOperator() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

std::optional<Batch> ScanOperator::Next() {
    if (batch_num_ >= metadata_.BatchesCnt()) {
        return std::nullopt;
    }
    Schema& schema = metadata_.GetSchema();
    size_t columns_cnt = schema.GetSize();
    std::vector<Column> res_columns;
    Schema res_schema;

    for (size_t i = 0; i < columns_cnt; ++i) {
        std::string column_name = schema.GetName(i);
        Type column_type = schema.GetType(i);
        int64_t rows_cnt = metadata_.GetRowsCnt()[batch_num_];
        if (!columns_.empty() && !columns_.contains(column_name)) {
            if (column_type == Type::string) {
                int64_t total = 0;
                fin_.read(reinterpret_cast<char*>(&total), sizeof(total));
                fin_.seekg(total, std::ios::cur);
            } else {
                int64_t total = rows_cnt * TypeSizeof(column_type);
                fin_.seekg(total, std::ios::cur);
            }
            continue;
        }
        res_schema.AddElement(schema.GetElement(i));
        auto column = ReadColumnFromColumnar(column_type, fin_, rows_cnt);
        if (!column) {
            return std::nullopt;
        }
        res_columns.push_back(std::move(*column));
    }
    ++batch_num_;
    return Batch(std::move(res_columns), std::move(res_schema));
}
