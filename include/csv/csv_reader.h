#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

struct CsvBatcher;

class CsvReader {
public:
    CsvReader() = default;
    ~CsvReader();

    CsvReader(const CsvReader&) = delete;
    CsvReader operator=(const CsvReader&) = delete;

    CsvReader(CsvReader&&) = default;
    CsvReader& operator=(CsvReader&&) = default;

    bool HasNext();

    std::expected<std::vector<std::string>, std::string> NextStr();

private:
    std::string filename_;
    std::ifstream fin_;
    bool crashed_ = false;

    friend std::expected<CsvReader, std::string> CreateCsvReader(const std::string& csv_filename);

    CsvReader(const std::string& filename);

    bool Open();
    friend struct CsvBatcher;

    bool IsCrashed();
};

std::expected<CsvReader, std::string> CreateCsvReader(const std::string& csv_filename);
