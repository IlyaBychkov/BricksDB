#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

class CSVReader {
public:
    CSVReader() = default;
    ~CSVReader();

    CSVReader(const CSVReader&) = delete;
    CSVReader operator=(const CSVReader&) = delete;

    CSVReader(CSVReader&&) = default;
    CSVReader& operator=(CSVReader&&) = default;

    bool Open();

    bool HasNext();

    bool IsCrashed();

    std::expected<std::vector<std::string>, std::string> NextStr();

private:
    CSVReader(const std::string& filename);
    friend std::expected<CSVReader, std::string> CreateCSVReader(const std::string& csv_filename);

    std::string filename_;
    std::ifstream fin_;
    bool crashed_ = false;
};

std::expected<CSVReader, std::string> CreateCSVReader(const std::string& csv_filename);
