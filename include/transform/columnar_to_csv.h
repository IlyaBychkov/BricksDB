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

    bool Transform();

private:
    std::string columnar_filename_;
    std::string scheme_filename_;
    std::string csv_filename_;

    std::ifstream fin_;
    CSVWriter csv_out_;

    bool Prepare();
    bool WriteBatchToCSV(const Batch& batch);
};