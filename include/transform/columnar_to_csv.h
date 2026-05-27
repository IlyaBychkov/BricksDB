#pragma once

#include <fstream>
#include <string>

#include "csv/csv_writer.h"

struct ColumnarToCsvTransformer {
public:
    ColumnarToCsvTransformer(const std::string& columnar_filename,
                             const std::string& schema_filename, const std::string& csv_filename);
    ~ColumnarToCsvTransformer();

    std::expected<void, std::string> Transform();

private:
    std::string columnar_filename_;
    std::string schema_filename_;
    std::string csv_filename_;

    std::ifstream fin_;
    CsvWriter csv_out_;

    std::expected<void, std::string> Prepare();
};
