#include "transform/metadata.h"

#include <cstdint>

#include "scheme/scheme.h"
#include "scheme/type.h"

// TODO: Написать функции WriteNum, WriteString, WriteVector и читалки к ним

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

std::expected<void, std::string> WriteMetadataToFile(Metadata metadata, std::ostream& fout) {
    try {
        int64_t sum = 0;
        int64_t scheme_size = metadata.GetScheme().GetSize();
        fout.write(reinterpret_cast<char*>(&scheme_size), sizeof(int64_t));
        sum += sizeof(int64_t);

        for (auto& elem : metadata.GetScheme().GetAllElements()) {
            int64_t type_int = static_cast<int64_t>(elem.GetType());
            fout.write(reinterpret_cast<char*>(&type_int), sizeof(int64_t));
            sum += sizeof(int64_t);

            int64_t name_size = elem.GetName().size();
            fout.write(reinterpret_cast<char*>(&name_size), sizeof(int64_t));
            sum += sizeof(int64_t);

            fout.write(elem.GetName().data(), name_size);
            sum += name_size;
        }

        int64_t row_groups = metadata.GetOffsets().size();
        fout.write(reinterpret_cast<char*>(&row_groups), sizeof(int64_t));
        sum += sizeof(int64_t);

        fout.write(reinterpret_cast<char*>(metadata.GetOffsets().data()),
                   row_groups * sizeof(int64_t));
        fout.write(reinterpret_cast<char*>(metadata.GetRowsCnt().data()),
                   row_groups * sizeof(int64_t));
        sum += 2 * row_groups * sizeof(int64_t);

        sum += sizeof(int64_t);
        fout.write(reinterpret_cast<char*>(&sum), sizeof(int64_t));
    } catch (...) {
        return std::unexpected("Failed to write metadata to file");
    }
    return {};
}

std::expected<Metadata, std::string> ReadMetadataFromFile(std::istream& fin) {
    try {
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
        Metadata metadata = Metadata(Scheme(elements));

        int64_t row_groups;
        fin.read(reinterpret_cast<char*>(&row_groups), sizeof(int64_t));

        metadata.GetOffsets().resize(row_groups);
        fin.read(reinterpret_cast<char*>(metadata.GetOffsets().data()),
                 row_groups * sizeof(int64_t));

        metadata.GetRowsCnt().resize(row_groups);
        fin.read(reinterpret_cast<char*>(metadata.GetRowsCnt().data()),
                 row_groups * sizeof(int64_t));
        return metadata;
    } catch (...) {
        return std::unexpected("Failed to read metadata from file");
    }
}