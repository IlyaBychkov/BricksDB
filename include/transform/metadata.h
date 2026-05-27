#pragma once

#include <istream>
#include <ostream>

#include "schema/schema.h"
struct Metadata {
public:
    Metadata() = default;
    Metadata(const Schema& schema) : schema_(schema) {
    }

    void AddRowGroup(int64_t offset, int64_t rows);

    Schema& GetSchema();
    std::vector<int64_t>& GetOffsets();
    std::vector<int64_t>& GetRowsCnt();

    size_t BatchesCnt() const;

private:
    Schema schema_;
    std::vector<int64_t> offsets_;
    std::vector<int64_t> rows_;
};

std::expected<void, std::string> WriteMetadataToFile(Metadata metadata, std::ostream& fout);
std::expected<Metadata, std::string> ReadMetadataFromFile(std::istream& fin);
