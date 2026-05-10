#include "csv_writer.h"

#include <chrono>

#include "time_transform.h"

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

std::expected<void, std::string> CSVWriter::WriteRow(const std::vector<std::string>& fields,
                                                     bool need_flush) {
    if (!fout_.is_open()) {
        crashed_ = true;
    }
    if (crashed_) {
        return std::unexpected("CSVWriter::WriteRow: File is not open (" + filename_ + ")");
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
        return std::unexpected("CSVWriter::WriteRow: Writer crashed while writing to " + filename_);
    }

    return {};
}

std::expected<void, std::string> CSVWriter::WriteBatch(const Batch& batch) {
    for (size_t i = 0; i < batch.RowsCnt(); ++i) {
        std::vector<std::string> row;
        for (size_t c = 0; c < batch.ColumnsCnt(); ++c) {
            Type t = batch.GetColumnType(c);
            if (t == Type::int64) {
                const auto& val = batch.GetColumn(c).GetValue<int64_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::int32) {
                const auto& val = batch.GetColumn(c).GetValue<int32_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::int16) {
                const auto& val = batch.GetColumn(c).GetValue<int16_t>(i);
                row.push_back(std::to_string(val));
            } else if (t == Type::string) {
                const auto& val = batch.GetColumn(c).GetValue<std::string>(i);
                row.push_back(val);
            } else if (t == Type::timestamp) {
                const auto& val = batch.GetColumn(c).GetValue<int64_t>(i);
                row.push_back(IntToTimestamp(val));
            } else if (t == Type::date) {
                const auto& val = batch.GetColumn(c).GetValue<int32_t>(i);
                row.push_back(IntToDate(val));
            } else {
                return std::unexpected(std::string("CSVWriter::WriteBatchToCSV: "
                                                   "Unsupported column type at column ") +
                                       std::to_string(c));
            }
        }
        auto res = WriteRow(row);
        if (!res) {
            return std::unexpected(
                std::string("CSVWriter::WriteBatchToCSV: CSVWriter WriteRow failed: ") +
                res.error());
        }
    }
    if (IsCrashed()) {
        return std::unexpected(
            std::string("CSVWriter::WriteBatchToCSV: CSVWriter crashed while writing to '") +
            filename_ + "'");
    }
    return {};
}

std::expected<CSVWriter, std::string> CreateCSVWriter(const std::string& csv_filename) {
    CSVWriter writer(csv_filename);
    if (!writer.Open()) {
        return std::unexpected("CreateCSVWriter: Failed to open CSV file for writing: " +
                               csv_filename);
    }
    return writer;
}
