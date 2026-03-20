#include "csv_reader.h"

CSVReader::CSVReader(const std::string& filename) : filename_(filename) {
}

CSVReader::~CSVReader() {
    if (fin_.is_open()) {
        fin_.close();
    }
}

bool CSVReader::Open() {
    fin_.open(filename_);
    return fin_.is_open();
}

bool CSVReader::IsCrashed() {
    return crashed_;
}

bool CSVReader::HasNext() {
    return fin_.is_open() && !crashed_ && fin_.peek() != EOF;
}

std::expected<std::vector<std::string>, std::string> CSVReader::NextStr() {
    if (crashed_) {
        return std::unexpected("CSVReader::NextStr: reader is crashed (" + filename_ + ")");
    }

    if (!fin_.is_open()) {
        return std::unexpected("CSVReader::NextStr: file is not open (" + filename_ + ")");
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
    return std::unexpected("CSVReader::NextStr: no more records in file (" + filename_ + ")");
}

std::expected<CSVReader, std::string> CreateCSVReader(const std::string& csv_filename) {
    CSVReader reader(csv_filename);
    if (!reader.Open()) {
        return std::unexpected("CreateCSVReader: Failed to open CSV file: " + csv_filename);
    }
    return reader;
}
