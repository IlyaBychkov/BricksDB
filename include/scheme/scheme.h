#pragma once

#include <string>
#include <vector>

#include "type.h"

struct SchemeElement {
public:
    SchemeElement(const std::string& name, Type type);

    const std::string& GetName() const;
    Type GetType() const;

    std::string ToString() const;

private:
    std::string name_;
    Type type_;
};

struct Scheme {
public:
    Scheme() = default;
    Scheme(const std::vector<SchemeElement>& values);

    bool ConstructFromFile(const std::string& filename);
    bool WriteToFile(const std::string& filename) const;

    size_t GetSize() const;

    const SchemeElement& GetElement(size_t ind) const;
    const std::vector<SchemeElement>& GetAllElements() const;
    const std::string& GetName(size_t ind) const;
    Type GetType(size_t ind) const;

    void AddElement(SchemeElement elem);

private:
    std::vector<SchemeElement> values_;
};
