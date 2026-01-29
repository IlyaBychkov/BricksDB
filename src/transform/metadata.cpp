#include "transform/metadata.h"

#include <cstdint>

bool Metadata::WriteToFile(std::ostream& fout) {
    int64_t scheme_size = scheme_.GetSize();
    fout.write(reinterpret_cast<char*>(&scheme_size), sizeof(int64_t));
    for (auto& elem : scheme_.GetAllElements()) {
        int64_t type_int = static_cast<int64_t>(elem.GetType());
        fout.write(reinterpret_cast<char*>(&type_int), sizeof(int64_t));
        int64_t name_size = elem.GetName().size();
        fout.write(reinterpret_cast<char*>(&name_size), sizeof(int64_t));
        fout.write(elem.GetName().data(), name_size);
    }

    int64_t row_groups = offsets_.size();
    fout.write(reinterpret_cast<char*>(&row_groups), sizeof(int64_t));
    fout.write(reinterpret_cast<char*>(offsets_.data()), row_groups * sizeof(int64_t));
    fout.write(reinterpret_cast<char*>(rows_.data()), row_groups * sizeof(int64_t));
    return fout.good();
}
