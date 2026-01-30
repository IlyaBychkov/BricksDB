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

bool Scheme::ConstructFromFile(const std::string& filename) {
    auto res = CreateCSVReader(filename);
    if (!res.has_value()) {
        return false;
    }
    CSVReader reader = std::move(res.value());

    while (reader.HasNext()) {
        auto tmp = reader.NextStr();
        if (!tmp.has_value()) {
            return false;
        }
        auto row = tmp.value();
        if (row.size() != 2) {
            return false;
        }
        Type type;
        if (row[1] == "int64") {
            type = Type::int64;
        } else if (row[1] == "string") {
            type = Type::string;
        } else {
            return false;
        }
        values_.emplace_back(row[0], type);
    }
    return !reader.IsCrashed();
}

bool Scheme::WriteToFile(const std::string& filename) const {
    auto tmp = CreateCSVWriter(filename);
    if (!tmp) {
        return false;
    }
    CSVWriter writer = std::move(tmp.value());
    for (const auto& elem : values_) {
        if (!writer.WriteRow({elem.GetName(), TypeToString(elem.GetType())})) {
            return false;
        }
    }
    return !writer.IsCrashed();
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
