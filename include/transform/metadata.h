#pragma once

#include <istream>
#include <ostream>

#include "scheme/scheme.h"
struct Metadata {
public:
    Metadata() = default;
    Metadata(const Scheme& scheme) : scheme_(scheme) {
    }

    bool WriteToFile(std::ostream& fout);
    bool ReadFromFile(std::istream& fin);

    void AddRowGroup(int64_t offset, int64_t rows);

    Scheme& GetScheme();
    std::vector<int64_t>& GetOffsets();
    std::vector<int64_t>& GetRowsCnt();

private:
    Scheme scheme_;
    std::vector<int64_t> offsets_;
    std::vector<int64_t> rows_;
};