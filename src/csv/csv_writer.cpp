#include "csv_writer.h"

#include <chrono>

#include "time_transform.h"

CsvWriter::CsvWriter(const std::string& filename) : filename_(filename) {
}

CsvWriter::~CsvWriter() {
    if (fout_.is_open()) {
        Flush();
        fout_.close();
    }
}

bool CsvWriter::Open() {
    fout_.open(filename_);
    if (!fout_.is_open()) {
        crashed_ = true;
        return false;
    }
    return true;
}

bool CsvWriter::IsCrashed() {
    return crashed_;
}

bool CsvWriter::Flush() {
    if (!fout_.is_open()) {
        crashed_ = true;
        return false;
    }
    fout_.flush();
    return true;
}

std::expected<void, std::string> CsvWriter::WriteRow(const std::vector<std::string>& fields,
                                                     bool need_flush) {
    if (!fout_.is_open()) {
        crashed_ = true;
    }
    if (crashed_) {
        return std::unexpected("CsvWriter::WriteRow: File is not open (" + filename_ + ")");
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
        return std::unexpected("CsvWriter::WriteRow: Writer crashed while writing to " + filename_);
    }

    return {};
}

std::expected<void, std::string> CsvWriter::WriteBatch(const Batch& batch) {
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
                return std::unexpected(std::string("CsvWriter::WriteBatchToCsv: "
                                                   "Unsupported column type at column ") +
                                       std::to_string(c));
            }
        }
        auto res = WriteRow(row);
        if (!res) {
            return std::unexpected(
                std::string("CsvWriter::WriteBatchToCsv: CsvWriter WriteRow failed: ") +
                res.error());
        }
    }
    if (IsCrashed()) {
        return std::unexpected(
            std::string("CsvWriter::WriteBatchToCsv: CsvWriter crashed while writing to '") +
            filename_ + "'");
    }
    return {};
}

std::expected<CsvWriter, std::string> CreateCsvWriter(const std::string& csv_filename) {
    CsvWriter writer(csv_filename);
    if (!writer.Open()) {
        return std::unexpected("CreateCsvWriter: Failed to open Csv file for writing: " +
                               csv_filename);
    }
    return writer;
}
