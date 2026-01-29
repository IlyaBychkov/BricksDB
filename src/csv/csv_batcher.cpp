#include "csv/csv_batcher.h"

std::expected<CSVBatcher, std::string> CreateCSVBatcher(const std::string& csv_file,
                                                        const std::string& scheme_file,
                                                        int64_t batch_max_size) {
    CSVReader reader(csv_file);
    if (!reader.Open()) {
        return std::unexpected("Failed to open CSV file: " + csv_file);
    }
    Scheme scheme;
    if (!scheme.ConstructFromFile(scheme_file)) {
        return std::unexpected("Failed to open scheme file: " + scheme_file);
    }
    return CSVBatcher(std::move(scheme), std::move(reader), batch_max_size);
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
            return std::unexpected("NextStr failed in CSVBatcher NextBatch");
        }
        auto row = tmp.value();
        if (row.size() != columns.size()) {
            return std::unexpected("Bad scheme or CSV in CSVBatcher NextBatch");
        }

        for (size_t i = 0; i < columns.size(); ++i) {
            if (scheme_.GetType(i) == Type::int64) {
                columns[i].Push<int64_t>(std::stoll(row[i]));
                current_batch_size += sizeof(int64_t);
            } else if (scheme_.GetType(i) == Type::string) {
                columns[i].Push<std::string>(row[i]);
                current_batch_size += row[i].size();
            } else {
                return std::unexpected("Unsupported type in CSVBatcher NextBatch");
            }
        }
    }

    if (reader_.IsCrashed()) {
        return std::unexpected("Reader crashed in CSVBatcher NextBatch");
    }

    return Batch(std::move(columns), scheme_);
}