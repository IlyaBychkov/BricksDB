#include "csv/csv_batcher.h"

#include <chrono>
#include <ctime>
#include <expected>

#include "csv/time_transform.h"

std::expected<CSVBatcher, std::string> CreateCSVBatcher(const std::string& csv_filename,
                                                        const std::string& scheme_file,
                                                        int64_t batch_max_size) {
    auto reader = CreateCSVReader(csv_filename);
    if (!reader.has_value()) {
        return std::unexpected("CreateCSVBatcher: CreateCSVReader failed for '" + csv_filename +
                               "': " + reader.error());
    }
    auto scheme = CreateSchemeFromCSV(scheme_file);
    if (!scheme.has_value()) {
        return std::unexpected("CreateCSVBatcher: CreateSchemeFromFile failed for '" + scheme_file +
                               "': " + scheme.error());
    }
    return CSVBatcher(std::move(*scheme), std::move(*reader), batch_max_size);
}

CSVBatcher::CSVBatcher(Scheme&& scheme, CSVReader&& reader, int64_t batch_max_size)
    : scheme_(std::move(scheme)), reader_(std::move(reader)), batch_max_size_(batch_max_size) {
}

bool CSVBatcher::IsCrashed() {
    return reader_.IsCrashed();
}

bool CSVBatcher::HasNextBatch() {
    return reader_.HasNext();
}

std::expected<Batch, std::string> CSVBatcher::NextBatch() {
    std::vector<Column> columns;
    for (size_t i = 0; i < scheme_.GetSize(); ++i) {
        columns.emplace_back(scheme_.GetType(i));
    }

    int64_t current_batch_size = 0;
    while (reader_.HasNext() && current_batch_size < batch_max_size_) {

        auto tmp = reader_.NextStr();
        if (!tmp.has_value()) {
            return std::unexpected(
                std::string("CSVBatcher::NextBatch: CSVReader NextStr failed: ") + tmp.error());
        }
        auto row = tmp.value();
        if (row.size() != columns.size()) {
            return std::unexpected("CSVBatcher::NextBatch: Bad scheme or CSV: expected " +
                                   std::to_string(columns.size()) + " columns, got " +
                                   std::to_string(row.size()));
        }

        for (size_t i = 0; i < columns.size(); ++i) {
            Type t = scheme_.GetType(i);
            if (t == Type::int64) {
                columns[i].Push<int64_t>(std::stoll(row[i]));
                current_batch_size += sizeof(int64_t);
            } else if (t == Type::int32) {
                columns[i].Push<int32_t>(static_cast<int32_t>(std::stoll(row[i])));
                current_batch_size += sizeof(int32_t);
            } else if (t == Type::int16) {
                columns[i].Push<int16_t>(static_cast<int16_t>(std::stoll(row[i])));
                current_batch_size += sizeof(int16_t);
            } else if (t == Type::string) {
                columns[i].Push<std::string>(row[i]);
                current_batch_size += row[i].size();
            } else if (t == Type::timestamp) {
                auto ts = TimestampToInt(row[i]);
                if (!ts.has_value()) {
                    return std::unexpected("CSVBatcher::NextBatch: " + ts.error());
                }
                columns[i].Push<int64_t>(ts.value());
                current_batch_size += sizeof(int64_t);
            } else if (t == Type::date) {
                auto dt = DateToInt(row[i]);
                if (!dt.has_value()) {
                    return std::unexpected("CSVBatcher::NextBatch: " + dt.error());
                }
                columns[i].Push<int32_t>(dt.value());
                current_batch_size += sizeof(int32_t);
            } else {
                return std::unexpected(
                    std::string("CSVBatcher::NextBatch: Unsupported type at column ") +
                    std::to_string(i));
            }
        }
    }

    if (reader_.IsCrashed()) {
        return std::unexpected("CSVBatcher::NextBatch: underlying CSVReader crashed");
    }

    return Batch(std::move(columns), scheme_);
}

const Scheme& CSVBatcher::GetScheme() {
    return scheme_;
}
