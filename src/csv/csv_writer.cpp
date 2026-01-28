#include "csv_writer.h"

CSVWriter::CSVWriter(const std::string& filename) : filename_(filename) {
}

CSVWriter::~CSVWriter() {
    if (fout_.is_open()) {
        Flush();
        fout_.close();
    }
}

bool CSVWriter::Open() {
    fout_.open(filename_);
    if (!fout_.is_open()) {
        crashed_ = true;
        return false;
    }
    return true;
}

bool CSVWriter::IsCrashed() {
    return crashed_;
}

bool CSVWriter::Flush() {
    if (!fout_.is_open()) {
        crashed_ = true;
        return false;
    }
    fout_.flush();
    return true;
}

bool CSVWriter::WriteRow(const std::vector<std::string>& fields, bool need_flush) {
    if (!fout_.is_open()) {
        crashed_ = true;
    }
    if (crashed_) {
        return false;
    }

    for (size_t i = 0; i < fields.size(); ++i) {
        const std::string& field = fields[i];

        bool needs_quoting = false;
        bool has_quotes = false;
        for (char c : field) {
            if (c == ',' || c == '"' || c == '\n') {
                needs_quoting = true;
                if (c == '"') {
                    has_quotes = true;
                }
            }
        }

        if (needs_quoting) {
            fout_ << '"';
        }

        if (has_quotes) {
            for (char c : field) {
                if (c == '"') {
                    fout_ << "\"\"";
                } else {
                    fout_ << c;
                }
            }
        } else {
            fout_ << field;
        }

        if (needs_quoting) {
            fout_ << '"';
        }

        if (i + 1 != fields.size()) {
            fout_ << ',';
        }
    }

    fout_ << '\n';

    if (need_flush) {
        Flush();
    }

    if (fout_.fail()) {
        crashed_ = true;
        return false;
    }

    return true;
}
