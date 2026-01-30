#include "transform/metadata.h"

#include <cstdint>

#include "scheme/scheme.h"
#include "scheme/type.h"

// TODO: Написать функции WriteNum, WriteString, WriteVector и читалки к ним

bool Metadata::WriteToFile(std::ostream& fout) {
    int64_t sum = 0;
    int64_t scheme_size = scheme_.GetSize();
    fout.write(reinterpret_cast<char*>(&scheme_size), sizeof(int64_t));
    sum += sizeof(int64_t);

    for (auto& elem : scheme_.GetAllElements()) {
        int64_t type_int = static_cast<int64_t>(elem.GetType());
        fout.write(reinterpret_cast<char*>(&type_int), sizeof(int64_t));
        sum += sizeof(int64_t);

        int64_t name_size = elem.GetName().size();
        fout.write(reinterpret_cast<char*>(&name_size), sizeof(int64_t));
        sum += sizeof(int64_t);

        fout.write(elem.GetName().data(), name_size);
        sum += name_size;
    }

    int64_t row_groups = offsets_.size();
    fout.write(reinterpret_cast<char*>(&row_groups), sizeof(int64_t));
    sum += sizeof(int64_t);

    fout.write(reinterpret_cast<char*>(offsets_.data()), row_groups * sizeof(int64_t));
    fout.write(reinterpret_cast<char*>(rows_.data()), row_groups * sizeof(int64_t));
    sum += 2 * row_groups * sizeof(int64_t);

    sum += sizeof(int64_t);
    fout.write(reinterpret_cast<char*>(&sum), sizeof(int64_t));

    return fout.good();
}

bool Metadata::ReadFromFile(std::istream& fin) {
    fin.seekg(-sizeof(int64_t), std::ios::end);
    int64_t meta_size;
    fin.read(reinterpret_cast<char*>(&meta_size), sizeof(int64_t));

    fin.seekg(-meta_size, std::ios::end);
    int64_t scheme_size;
    fin.read(reinterpret_cast<char*>(&scheme_size), sizeof(int64_t));

    std::vector<SchemeElement> elements;
    for (int64_t i = 0; i < scheme_size; ++i) {
        int64_t type_int;
        fin.read(reinterpret_cast<char*>(&type_int), sizeof(int64_t));

        int64_t name_size;
        fin.read(reinterpret_cast<char*>(&name_size), sizeof(int64_t));

        std::string name(name_size, '\0');
        fin.read(name.data(), name_size);

        elements.emplace_back(name, static_cast<Type>(type_int));
    }
    scheme_ = Scheme(elements);

    int64_t row_groups;
    fin.read(reinterpret_cast<char*>(&row_groups), sizeof(int64_t));

    offsets_.resize(row_groups);
    fin.read(reinterpret_cast<char*>(offsets_.data()), row_groups * sizeof(int64_t));

    rows_.resize(row_groups);
    fin.read(reinterpret_cast<char*>(rows_.data()), row_groups * sizeof(int64_t));

    return fin.good();
}

void Metadata::AddRowGroup(int64_t offset, int64_t rows) {
    offsets_.push_back(offset);
    rows_.push_back(rows);
}

Scheme& Metadata::GetScheme() {
    return scheme_;
}
std::vector<int64_t>& Metadata::GetOffsets() {
    return offsets_;
}
std::vector<int64_t>& Metadata::GetRowsCnt() {
    return rows_;
}