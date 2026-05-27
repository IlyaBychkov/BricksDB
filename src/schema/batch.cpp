#include "schema/batch.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "csv_writer.h"

Batch::Batch(const std::vector<Column>& data, const Schema& schema) : data_(data), schema_(schema) {
    if (!Validate()) {
        throw std::runtime_error("Invalid Batch construction");
    }
}

Batch::Batch(std::vector<Column>&& data, const Schema& schema)
    : data_(std::move(data)), schema_(schema) {
    if (!Validate()) {
        throw std::runtime_error("Invalid Batch construction");
    }
}

std::expected<void, std::string> Batch::Validate() const {
    if (data_.size() != schema_.GetSize()) {
        return std::unexpected(
            "Batch::Validate: Column count mismatch: data=" + std::to_string(data_.size()) +
            ", schema=" + std::to_string(schema_.GetSize()));
    }
    for (size_t i = 0; i < data_.size(); ++i) {
        if (data_[i].GetType() != schema_.GetType(i)) {
            return std::unexpected("Batch::Validate: Type mismatch at index " + std::to_string(i) +
                                   ": expected " + TypeToString(schema_.GetType(i)) + ", but got " +
                                   TypeToString(data_[i].GetType()));
        }
        if (data_[i].GetSize() != data_[0].GetSize()) {
            return std::unexpected(
                "Batch::Validate: Column size mismatch at index " + std::to_string(i) +
                ": size is " + std::to_string(data_[i].GetSize()) + ", but first column size is " +
                std::to_string(data_[0].GetSize()));
        }
    }
    return {};
}

size_t Batch::ColumnsCnt() const {
    return data_.size();
}

size_t Batch::RowsCnt() const {
    if (data_.empty()) {
        return 0;
    }
    return data_[0].GetSize();
}

Schema& Batch::GetSchema() {
    return schema_;
}
const Schema& Batch::GetSchema() const {
    return schema_;
}

Type Batch::GetColumnType(size_t ind) const {
    return schema_.GetType(ind);
}

const std::string& Batch::GetColumnName(size_t ind) const {
    return schema_.GetName(ind);
}

Column& Batch::GetColumn(size_t ind) {
    return data_.at(ind);
}
const Column& Batch::GetColumn(size_t ind) const {
    return data_.at(ind);
}

std::vector<Column>& Batch::GetAllColumns() {
    return data_;
}
const std::vector<Column>& Batch::GetAllColumns() const {
    return data_;
}

Column& Batch::GetColumn(const std::string& name) {
    for (size_t i = 0; i < ColumnsCnt(); ++i) {
        if (GetColumnName(i) == name) {
            return data_[i];
        }
    }
    throw std::runtime_error("Column not found: " + name);
}

const Column& Batch::GetColumn(const std::string& name) const {
    for (size_t i = 0; i < ColumnsCnt(); ++i) {
        if (GetColumnName(i) == name) {
            return data_[i];
        }
    }
    throw std::runtime_error("Column not found: " + name);
}

void Batch::AddColumn(const Column& columnn, const SchemaElement& se) {
    data_.push_back(columnn);
    schema_.AddElement(se);
}

void Batch::Merge(Batch&& other) {
    if (ColumnsCnt() != other.ColumnsCnt()) {
        throw std::runtime_error("Batch::Merge: Column count mismatch");
    }
    // TODO: GetSchema() == other.GetSchema()
    for (size_t i = 0; i < ColumnsCnt(); ++i) {
        std::visit(
            [col = other.GetColumn(i).Value()]<typename T>(std::vector<T>& vec) {
                auto& src = std::get<std::vector<T>>(col);
                vec.insert(vec.end(), src.begin(), src.end());
            },
            data_[i].Value());
    }
}

void Batch::ClearValues() {
    for (auto& column : data_) {
        std::visit([](auto& vec) { vec.clear(); }, column.Value());
    }
}

std::expected<Batch, std::string> CreateBatchFromFile(const Schema& schema, std::ifstream& fin,
                                                      int64_t rows_cnt) {
    std::vector<Column> data;
    for (size_t i = 0; i < schema.GetSize(); ++i) {
        auto res = ReadColumnFromColumnar(schema.GetType(i), fin, rows_cnt);
        if (!res) {
            throw std::runtime_error(res.error());
        }
        data.push_back(std::move(*res));
    }

    Batch batch(std::move(data), schema);
    auto res = batch.Validate();
    if (!res) {
        throw std::runtime_error(res.error());
    }

    return batch;
}

std::expected<void, std::string> WriteBatchToCsv(const Batch& batch, const std::string& filename) {
    auto t = CreateCsvWriter(filename);
    if (!t.has_value()) {
        return std::unexpected("WriteBatchToCsv: Failed to create Csv writer: " + t.error());
    }
    CsvWriter writer = std::move(*t);
    auto res = writer.WriteBatch(batch);
    if (!res) {
        return std::unexpected("WriteBatchToCsv: Failed to write batch to Csv: " + res.error());
    }
    return {};
}
