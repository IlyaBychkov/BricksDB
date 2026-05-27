#pragma once

#include <cstdint>
#include <expected>

#include "../schema/batch.h"
#include "../schema/schema.h"
#include "csv_reader.h"

struct CsvBatcher {
public:
    CsvBatcher() = default;
    CsvBatcher(Schema&& schema, CsvReader&& reader, int64_t max_batch_size_bytes);

    CsvBatcher(const CsvBatcher&) = delete;
    CsvBatcher operator=(const CsvBatcher&) = delete;

    CsvBatcher(CsvBatcher&&) = default;
    CsvBatcher& operator=(CsvBatcher&&) = default;

    bool HasNextBatch();
    std::expected<Batch, std::string> NextBatch();

    const Schema& GetSchema();

private:
    Schema schema_;
    CsvReader reader_;
    int64_t max_batch_size_bytes_;
};

std::expected<CsvBatcher, std::string> CreateCsvBatcher(const std::string& csv_file,
                                                        const std::string& schema_file,
                                                        int64_t max_batch_size_bytes = 1024 * 1024 *
                                                                                       512);
