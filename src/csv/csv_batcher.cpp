#include "csv/csv_batcher.h"

#include <ctime>
#include <expected>

#include "csv/time_transform.h"

std::expected<CsvBatcher, std::string> CreateCsvBatcher(const std::string& csv_filename,
                                                        const std::string& schema_file,
                                                        int64_t max_batch_size_bytes) {
    auto reader = CreateCsvReader(csv_filename);
    if (!reader.has_value()) {
        return std::unexpected("CreateCsvBatcher: CreateCsvReader failed for '" + csv_filename +
                               "': " + reader.error());
    }
    auto schema = CreateSchemaFromCsv(schema_file);
    if (!schema.has_value()) {
        return std::unexpected("CreateCsvBatcher: CreateSchemaFromFile failed for '" + schema_file +
                               "': " + schema.error());
    }
    return CsvBatcher(std::move(*schema), std::move(*reader), max_batch_size_bytes);
}

CsvBatcher::CsvBatcher(Schema&& schema, CsvReader&& reader, int64_t max_batch_size_bytes)
    : schema_(std::move(schema)),
      reader_(std::move(reader)),
      max_batch_size_bytes_(max_batch_size_bytes) {
}

bool CsvBatcher::HasNextBatch() {
    return reader_.HasNext();
}

std::expected<Batch, std::string> CsvBatcher::NextBatch() {
    std::vector<Column> columns;
    for (size_t i = 0; i < schema_.GetSize(); ++i) {
        columns.emplace_back(schema_.GetType(i));
    }

    int64_t current_batch_size = 0;
    while (reader_.HasNext() && current_batch_size < max_batch_size_bytes_) {

        auto tmp = reader_.NextStr();
        if (!tmp.has_value()) {
            return std::unexpected(
                std::string("CsvBatcher::NextBatch: CsvReader NextStr failed: ") + tmp.error());
        }
        auto row = tmp.value();
        if (row.size() != columns.size()) {
            return std::unexpected("CsvBatcher::NextBatch: Bad schema or Csv: expected " +
                                   std::to_string(columns.size()) + " columns, got " +
                                   std::to_string(row.size()));
        }

        for (size_t i = 0; i < columns.size(); ++i) {
            Type t = schema_.GetType(i);
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
                    return std::unexpected("CsvBatcher::NextBatch: " + ts.error());
                }
                columns[i].Push<int64_t>(ts.value());
                current_batch_size += sizeof(int64_t);
            } else if (t == Type::date) {
                auto dt = DateToInt(row[i]);
                if (!dt.has_value()) {
                    return std::unexpected("CsvBatcher::NextBatch: " + dt.error());
                }
                columns[i].Push<int32_t>(dt.value());
                current_batch_size += sizeof(int32_t);
            } else {
                return std::unexpected(
                    std::string("CsvBatcher::NextBatch: Unsupported type at column ") +
                    std::to_string(i));
            }
        }
    }

    if (reader_.IsCrashed()) {
        return std::unexpected("CsvBatcher::NextBatch: underlying CsvReader crashed");
    }

    return Batch(std::move(columns), schema_);
}

const Schema& CsvBatcher::GetSchema() {
    return schema_;
}
