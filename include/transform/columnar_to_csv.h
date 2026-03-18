#pragma once

#include <fstream>
#include <string>

#include "csv/csv_writer.h"
#include "scheme/batch.h"

struct ColumnarToCSVTransformer {
public:
    ColumnarToCSVTransformer(const std::string& columnar_filename,
                             const std::string& scheme_filename, const std::string& csv_filename);
    ~ColumnarToCSVTransformer();

    std::expected<void, std::string> Transform();

private:
    std::string columnar_filename_;
    std::string scheme_filename_;
    std::string csv_filename_;

    std::ifstream fin_;
    CSVWriter csv_out_;

    std::expected<void, std::string> Prepare();
    std::expected<void, std::string> WriteBatchToCSV(const Batch& batch);
};