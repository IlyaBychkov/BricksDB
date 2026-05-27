#pragma once

#include <string>
#include <vector>

#include "type.h"

struct SchemaElement {
public:
    SchemaElement(const std::string& name, Type type);

    const std::string& GetName() const;
    Type GetType() const;

    std::string ToString() const;

private:
    std::string name_;
    Type type_;
};

struct Schema {
public:
    Schema() = default;
    Schema(const std::vector<SchemaElement>& values);

    size_t GetSize() const;

    const SchemaElement& GetElement(size_t ind) const;
    const std::vector<SchemaElement>& GetAllElements() const;
    const std::string& GetName(size_t ind) const;
    Type GetType(size_t ind) const;

    void AddElement(SchemaElement elem);

private:
    std::vector<SchemaElement> values_;
};

std::expected<Schema, std::string> CreateSchemaFromCsv(const std::string& filename);

std::expected<void, std::string> WriteSchemaToCsv(Schema schema, const std::string& filename);
