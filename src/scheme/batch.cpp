#include "scheme/batch.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>

Batch::Batch(const std::vector<Column>& data, const Scheme& scheme) : data_(data), scheme_(scheme) {
    if (!Validate()) {
        throw std::runtime_error("Invalid Batch construction");
    }
}

Batch::Batch(std::vector<Column>&& data, const Scheme& scheme)
    : data_(std::move(data)), scheme_(scheme) {
    if (!Validate()) {
        throw std::runtime_error("Invalid Batch construction");
    }
}

std::expected<void, std::string> Batch::Validate() const {
    if (data_.size() != scheme_.GetSize()) {
        return std::unexpected("Column count mismatch");
    }
    for (size_t i = 0; i < data_.size(); ++i) {
        if (data_[i].GetType() != scheme_.GetType(i) || data_[i].GetSize() != data_[0].GetSize()) {
            return std::unexpected("Column data mismatch");
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

Scheme& Batch::GetScheme() {
    return scheme_;
}
const Scheme& Batch::GetScheme() const {
    return scheme_;
}

Type Batch::GetColumnType(size_t ind) const {
    return scheme_.GetType(ind);
}

const std::string& Batch::GetColumnName(size_t ind) const {
    return scheme_.GetName(ind);
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

void Batch::AddColumn(const Column& columnn, const SchemeElement& se) {
    data_.push_back(columnn);
    scheme_.AddElement(se);
}

std::expected<Batch, std::string> CreateBatchFromFile(const Scheme& scheme, std::ifstream& fin,
                                                      int64_t rows_cnt) {
    std::vector<Column> data;
    for (size_t i = 0; i < scheme.GetSize(); ++i) {
        auto res = ReadColumnFromColumnar(scheme.GetType(i), fin, rows_cnt);
        if (!res) {
            throw std::runtime_error(res.error());
        }
        data.push_back(std::move(*res));
    }

    Batch batch(std::move(data), scheme);
    auto res = batch.Validate();
    if (!res) {
        throw std::runtime_error(res.error());
    }

    return batch;
}