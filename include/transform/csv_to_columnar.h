#pragma once

#include <cstdint>
#include <fstream>

#include "../csv/csv_batcher.h"

struct CsvToColumnarTransformer {
public:
    CsvToColumnarTransformer(const std::string& csv_filename, const std::string& schema_filename,
                             const std::string& columnar_filename,
                             int64_t max_batch_size_bytes = 1024 * 1024 * 512);
    ~CsvToColumnarTransformer();

    std::expected<void, std::string> Transform();

private:
    std::string csv_filename_;
    std::string schema_filename_;
    int64_t max_batch_size_bytes_;

    std::string columnar_filename_;

    CsvBatcher batcher_;
    std::ofstream fout_;

    std::expected<void, std::string> Prepare();
    std::expected<void, std::string> WriteBatch(const Batch& batch);
};
