#pragma once

#include <fstream>
#include <string>
#include <vector>

class CSVWriter {
public:
    CSVWriter(const std::string& filename);

    ~CSVWriter();

    CSVWriter(const CSVWriter&) = delete;
    CSVWriter operator=(const CSVWriter&) = delete;

    CSVWriter(CSVWriter&&) = default;
    CSVWriter& operator=(CSVWriter&&) = default;

    bool Open();

    bool Crashed();

    bool Flush();

    bool WriteStr(const std::vector<std::string>& fields, bool need_flush);

private:
    std::string filename_;
    std::ofstream fout_;
    bool crashed_ = false;
};
