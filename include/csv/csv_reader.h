#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>

class CSVReader {
public:
    CSVReader(const std::string& filename);

    ~CSVReader();

    CSVReader(const CSVReader&) = delete;
    CSVReader operator=(const CSVReader&) = delete;

    CSVReader(CSVReader&&) = default;
    CSVReader& operator=(CSVReader&&) = default;

    bool Open();

    bool HasNext();

    std::expected<std::vector<std::string>, std::string> NextStr();

private:
    std::string filename_;
    std::ifstream fin_;
    bool crashed_ = false;
};
