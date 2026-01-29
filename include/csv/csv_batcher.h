#include <cstdint>
#include <expected>

#include "../scheme/batch.h"
#include "../scheme/scheme.h"
#include "csv_reader.h"

struct CSVBatcher {
public:
    CSVBatcher(Scheme&& scheme, CSVReader&& reader, int64_t batch_max_size);

    CSVBatcher(const CSVBatcher&) = delete;
    CSVBatcher operator=(const CSVBatcher&) = delete;

    CSVBatcher(CSVBatcher&&) = default;
    CSVBatcher& operator=(CSVBatcher&&) = default;

    bool IsCrashed();
    bool HasNextBatch();
    std::expected<Batch, std::string> NextBatch();

private:
    Scheme scheme_;
    CSVReader reader_;
    int64_t batch_max_size_;  // in bytes
};

std::expected<CSVBatcher, std::string> CreateCSVBatcher(const std::string& csv_file,
                                                        const std::string& scheme_file,
                                                        int64_t batch_max_size = 1024 * 1024 * 512);