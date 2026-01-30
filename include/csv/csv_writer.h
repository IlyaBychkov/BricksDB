#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

class CSVWriter {
public:
    CSVWriter() = default;
    ~CSVWriter();

    CSVWriter(const CSVWriter&) = delete;
    CSVWriter operator=(const CSVWriter&) = delete;

    CSVWriter(CSVWriter&&) = default;
    CSVWriter& operator=(CSVWriter&&) = default;

    bool IsCrashed();

    bool Flush();

    bool WriteRow(const std::vector<std::string>& fields, bool need_flush = false);

private:
    std::string filename_;
    std::ofstream fout_;
    bool crashed_ = false;

    friend std::expected<CSVWriter, std::string> CreateCSVWriter(const std::string& csv_filename);

    CSVWriter(const std::string& filename);
    bool Open();
};

std::expected<CSVWriter, std::string> CreateCSVWriter(const std::string& csv_filename);
