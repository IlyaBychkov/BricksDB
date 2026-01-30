#pragma once

#include <cstdint>
#include <vector>

#include "column.h"
#include "scheme.h"

struct Batch {
public:
    Batch(const std::vector<Column>& data, const Scheme& scheme);
    Batch(std::vector<Column>&& data, const Scheme& scheme);
    Batch(const Scheme& scheme, std::ifstream& fin, int64_t rows_cnt);

    Batch(const Batch&) = default;
    Batch& operator=(const Batch&) = default;

    Batch(Batch&&) = default;
    Batch& operator=(Batch&&) = default;

    bool Validate() const;

    size_t ColumnsCnt() const;
    size_t RowsCnt() const;

    const Scheme& GetScheme() const;

    Type GetColumnType(size_t ind) const;
    const std::string& GetColumnName(size_t ind) const;

    Column& GetColumn(size_t ind);
    const Column& GetColumn(size_t ind) const;
    std::vector<Column>& GetAllColumns();
    const std::vector<Column>& GetAllColumns() const;

    void AddColumn(const Column& columnn, const SchemeElement& se);

private:
    std::vector<Column> data_;
    Scheme scheme_;
};