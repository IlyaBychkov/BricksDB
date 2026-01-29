#pragma once

#include <ostream>

#include "scheme/scheme.h"
struct Metadata {
public:
    Metadata() = default;
    Metadata(const Scheme& scheme) : scheme_(scheme) {
    }
    Metadata(const Scheme& scheme, const std::vector<int64_t>& offsets)
        : scheme_(scheme), offsets_(offsets) {
    }

    bool WriteToFile(std::ostream& fout);

    void AddRowGroup(int64_t offset, int64_t rows) {
        offsets_.push_back(offset);
        rows_.push_back(rows);
    }

private:
    Scheme scheme_;
    std::vector<int64_t> offsets_;
    std::vector<int64_t> rows_;
};