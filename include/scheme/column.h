#pragma once

#include <stdexcept>
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
    void Push(const T& v) {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on Push");
        }
        std::get<Vec>(value_).push_back(v);
    }

    template <typename T>
    std::vector<T>& GetVector() {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on GetVector");
        }
        return std::get<Vec>(value_);
    }

    ColumnValue& Value();
    const ColumnValue& Value() const;

    bool WriteToFile(std::ofstream& fout) const;

private:
    Type type_;
    ColumnValue value_;
};
