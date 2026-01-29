#pragma once

#include <string>
#include <variant>
#include <vector>

#include "type.h"

struct Column {
public:
    using ColumnValue = std::variant<std::vector<int64_t>, std::vector<std::string>>;

    Column(Type type);

    Type GetType() const;
    size_t GetSize() const;

    template <typename T>
    void Push(const T& v);

    template <typename T>
    T& At(size_t ind);

    template <typename T>
    const T& At(size_t ind) const;

    template <typename T>
    std::vector<T>& GetVector();

    template <typename T>
    const std::vector<T>& GetVector() const;

    ColumnValue& Value();
    const ColumnValue& Value() const;

private:
    Type type_;
    ColumnValue value_;
};
