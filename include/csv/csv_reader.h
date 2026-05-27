#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

class CsvReader {
public:
    CsvReader() = default;
    ~CsvReader();

    CsvReader(const CsvReader&) = delete;
    CsvReader operator=(const CsvReader&) = delete;

    CsvReader(CsvReader&&) = default;
    CsvReader& operator=(CsvReader&&) = default;

    bool HasNext();

    bool IsCrashed();

    std::expected<std::vector<std::string>, std::string> NextStr();

private:
    std::string filename_;
    std::ifstream fin_;
    bool crashed_ = false;

    friend std::expected<CsvReader, std::string> CreateCsvReader(const std::string& csv_filename);

    CsvReader(const std::string& filename);

    bool Open();
};

std::expected<CsvReader, std::string> CreateCsvReader(const std::string& csv_filename);
