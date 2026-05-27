#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

#include "../schema/batch.h"

class CsvWriter {
public:
    CsvWriter() = default;
    ~CsvWriter();

    CsvWriter(const CsvWriter&) = delete;
    CsvWriter operator=(const CsvWriter&) = delete;

    CsvWriter(CsvWriter&&) = default;
    CsvWriter& operator=(CsvWriter&&) = default;

    bool Flush();

    std::expected<void, std::string> WriteRow(const std::vector<std::string>& fields,
                                              bool need_flush = false);

    std::expected<void, std::string> WriteBatch(const Batch& batch);

private:
    friend std::expected<void, std::string> WriteSchemaToCsv(Schema schema,
                                                             const std::string& filename);

    std::string filename_;
    std::ofstream fout_;
    bool crashed_ = false;

    friend std::expected<CsvWriter, std::string> CreateCsvWriter(const std::string& csv_filename);

    CsvWriter(const std::string& filename);

    bool Open();

    bool IsCrashed();
};

std::expected<CsvWriter, std::string> CreateCsvWriter(const std::string& csv_filename);
