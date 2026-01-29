#pragma once

#include <vector>

#include "column.h"
#include "scheme.h"

struct Batch {
public:
    Batch(const std::vector<Column>& data, const Scheme& scheme);
    Batch(std::vector<Column>&& data, const Scheme& scheme);

    bool Validate() const;

    size_t GetSize() const;

    const Scheme& GetScheme() const;

    Type GetColumnType(size_t ind) const;
    const std::string& GetColumnName(size_t ind) const;

    Column& GetColumn(size_t ind);
    const Column& GetColumn(size_t ind) const;

    void AddColumn(const Column& columnn, const SchemeElement& se);

private:
    std::vector<Column> data_;
    Scheme scheme_;
};