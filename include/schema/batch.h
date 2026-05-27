#pragma once

#include <cstdint>
#include <expected>
#include <vector>

#include "column.h"
#include "schema.h"

struct Batch {
public:
    Batch(const std::vector<Column>& data, const Schema& schema);
    Batch(std::vector<Column>&& data, const Schema& schema);

    Batch(const Batch&) = default;
    Batch& operator=(const Batch&) = default;

    Batch(Batch&&) = default;
    Batch& operator=(Batch&&) = default;

    std::expected<void, std::string> Validate() const;

    size_t ColumnsCnt() const;
    size_t RowsCnt() const;

    Schema& GetSchema();
    const Schema& GetSchema() const;

    Type GetColumnType(size_t ind) const;
    const std::string& GetColumnName(size_t ind) const;

    Column& GetColumn(size_t ind);
    const Column& GetColumn(size_t ind) const;
    std::vector<Column>& GetAllColumns();
    const std::vector<Column>& GetAllColumns() const;

    Column& GetColumn(const std::string& name);
    const Column& GetColumn(const std::string& name) const;

    void AddColumn(const Column& columnn, const SchemaElement& se);

    void Merge(Batch&& other);

    void ClearValues();

private:
    std::vector<Column> data_;
    Schema schema_;
};

std::expected<Batch, std::string> CreateBatchFromFile(const Schema& schema, std::ifstream& fin,
                                                      int64_t rows_cnt);

std::expected<void, std::string> WriteBatchToCsv(const Batch& batch, const std::string& filename);
