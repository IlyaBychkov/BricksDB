#pragma once

#include <cstdint>
#include <fstream>

#include "../csv/csv_batcher.h"

struct CSVToColumnarTransformer {
public:
    CSVToColumnarTransformer(const std::string& csv_filename, const std::string& scheme_filename,
                             const std::string& columnar_filename,
                             int64_t batch_max_size = 1024 * 1024 * 512);
    ~CSVToColumnarTransformer();

    bool Transform();

private:
    std::string csv_filename_;
    std::string scheme_filename_;
    int64_t batch_max_size_;

    std::string columnar_filename_;

    CSVBatcher batcher_;
    std::ofstream fout_;

    bool Prepare();
    bool WriteBatch(const Batch& batch);
};