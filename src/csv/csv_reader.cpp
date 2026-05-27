#include "csv_reader.h"

CsvReader::CsvReader(const std::string& filename) : filename_(filename) {
}

CsvReader::~CsvReader() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

bool CsvReader::Open() {
    fin_.open(filename_);
    return fin_.is_open();
}

bool CsvReader::IsCrashed() {
    return crashed_;
}

bool CsvReader::HasNext() {
    return fin_.is_open() && !crashed_ && fin_.peek() != EOF;
}

std::expected<std::vector<std::string>, std::string> CsvReader::NextStr() {
    if (crashed_) {
        return std::unexpected("CsvReader::NextStr: reader is crashed (" + filename_ + ")");
    }

    if (!fin_.is_open()) {
        return std::unexpected("CsvReader::NextStr: file is not open (" + filename_ + ")");
    }

    std::vector<std::string> fields;
    std::string fld;
    bool in_quotes = false;
    char c;

    while (fin_.get(c)) {
        if (c == '"') {
            if (in_quotes && fin_.peek() == '"') {
                fin_.get();
                fld += '"';
            } else {
                in_quotes = !in_quotes;
            }
        } else if (c == ',' && !in_quotes) {
            fields.push_back(fld);
            fld.clear();
        } else if (c == '\n' && !in_quotes) {
            fields.push_back(fld);
            return fields;
        } else {
            fld += c;
        }
    }

    if (!fld.empty() || !fields.empty()) {
        if (!fld.empty()) {
            fields.push_back(fld);
        }
        return fields;
    }

    crashed_ = true;
    return std::unexpected("CsvReader::NextStr: no more records in file (" + filename_ + ")");
}

std::expected<CsvReader, std::string> CreateCsvReader(const std::string& csv_filename) {
    CsvReader reader(csv_filename);
    if (!reader.Open()) {
        return std::unexpected("CreateCsvReader: Failed to open Csv file: " + csv_filename);
    }
    return reader;
}
