#include "schema/schema.h"

#include "csv/csv_reader.h"
#include "csv/csv_writer.h"

SchemaElement::SchemaElement(const std::string& name, Type type) : name_(name), type_(type) {
}

const std::string& SchemaElement::GetName() const {
    return name_;
}

Type SchemaElement::GetType() const {
    return type_;
}

std::string SchemaElement::ToString() const {
    return name_ + "," + TypeToString(type_);
}

Schema::Schema(const std::vector<SchemaElement>& values) : values_(values) {
}

size_t Schema::GetSize() const {
    return values_.size();
}

const SchemaElement& Schema::GetElement(size_t ind) const {
    return values_[ind];
}

const std::vector<SchemaElement>& Schema::GetAllElements() const {
    return values_;
}

const std::string& Schema::GetName(size_t ind) const {
    return values_[ind].GetName();
}

Type Schema::GetType(size_t ind) const {
    return values_[ind].GetType();
}

void Schema::AddElement(SchemaElement elem) {
    values_.push_back(std::move(elem));
}

std::expected<Schema, std::string> CreateSchemaFromCsv(const std::string& filename) {
    auto res = CreateCsvReader(filename);
    if (!res.has_value()) {
        return std::unexpected(std::string("CreateSchemaFromFile: CreateCsvReader failed for '") +
                               filename + "': " + res.error());
    }
    CsvReader reader = std::move(res.value());
    Schema schema;

    while (reader.HasNext()) {
        auto tmp = reader.NextStr();
        if (!tmp.has_value()) {
            return std::unexpected(
                std::string("CreateSchemaFromFile: CsvReader NextStr failed for '") + filename +
                "': " + tmp.error());
        }
        auto row = tmp.value();
        if (row.size() != 2) {
            return std::unexpected(std::string("CreateSchemaFromFile: Invalid row size in '") +
                                   filename + "'");
        }
        auto tmp_type = StringToType(row[1]);
        if (!tmp_type.has_value()) {
            return std::unexpected(std::string("CreateSchemaFromFile: Unknown type in '") +
                                   filename + "' for value '" + row[1] + "': " + tmp_type.error());
        }
        schema.AddElement(SchemaElement(row[0], tmp_type.value()));
    }
    return schema;
}

std::expected<void, std::string> WriteSchemaToCsv(Schema schema, const std::string& filename) {
    auto wrirer_tmp = CreateCsvWriter(filename);
    if (!wrirer_tmp) {
        return std::unexpected(std::string("WriteSchemaToFile: CreateCsvWriter failed for '") +
                               filename + "': " + wrirer_tmp.error());
    }
    CsvWriter writer = std::move(*wrirer_tmp);
    for (const auto& elem : schema.GetAllElements()) {
        auto res = writer.WriteRow({elem.GetName(), TypeToString(elem.GetType())});
        if (!res) {
            return std::unexpected(std::string("WriteSchemaToFile: failed writing element '") +
                                   elem.GetName() + "' to '" + filename + "': " + res.error());
        }
    }
    if (writer.IsCrashed()) {
        return std::unexpected(
            std::string("WriteSchemaToFile: CsvWriter crashed while writing to '") + filename +
            "'");
    }
    return {};
}
