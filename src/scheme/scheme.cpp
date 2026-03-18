#include "scheme/scheme.h"

#include "csv/csv_reader.h"
#include "csv/csv_writer.h"

SchemeElement::SchemeElement(const std::string& name, Type type) : name_(name), type_(type) {
}

const std::string& SchemeElement::GetName() const {
    return name_;
}

Type SchemeElement::GetType() const {
    return type_;
}

std::string SchemeElement::ToString() const {
    return name_ + "," + TypeToString(type_);
}

Scheme::Scheme(const std::vector<SchemeElement>& values) : values_(values) {
}

size_t Scheme::GetSize() const {
    return values_.size();
}

const SchemeElement& Scheme::GetElement(size_t ind) const {
    return values_[ind];
}

const std::vector<SchemeElement>& Scheme::GetAllElements() const {
    return values_;
}

const std::string& Scheme::GetName(size_t ind) const {
    return values_[ind].GetName();
}

Type Scheme::GetType(size_t ind) const {
    return values_[ind].GetType();
}

void Scheme::AddElement(SchemeElement elem) {
    values_.push_back(std::move(elem));
}

std::expected<Scheme, std::string> CreateSchemeFromFile(const std::string& filename) {
    auto res = CreateCSVReader(filename);
    if (!res.has_value()) {
        return std::unexpected(res.error());
    }
    CSVReader reader = std::move(res.value());
    Scheme scheme;

    while (reader.HasNext()) {
        auto tmp = reader.NextStr();
        if (!tmp.has_value()) {
            return std::unexpected(tmp.error());
        }
        auto row = tmp.value();
        if (row.size() != 2) {
            return std::unexpected("Invalid row size");
        }
        auto tmp_type = StringToType(row[1]);
        if (!tmp_type.has_value()) {
            return std::unexpected(tmp_type.error());
        }
        scheme.AddElement(SchemeElement(row[0], tmp_type.value()));
    }
    return scheme;
}

std::expected<void, std::string> WriteSchemeToFile(Scheme scheme, const std::string& filename) {
    auto wrirer_tmp = CreateCSVWriter(filename);
    if (!wrirer_tmp) {
        return std::unexpected(wrirer_tmp.error());
    }
    CSVWriter writer = std::move(*wrirer_tmp);
    for (const auto& elem : scheme.GetAllElements()) {
        auto res = writer.WriteRow({elem.GetName(), TypeToString(elem.GetType())});
        if (!res) {
            return std::unexpected(res.error());
        }
    }
    if (writer.IsCrashed()) {
        return std::unexpected("Writer crashed");
    }
    return {};
}