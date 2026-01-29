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
    T& At(size_t ind) {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on At");
        }
        return std::get<Vec>(value_).at(ind);
    }

    template <typename T>
    const T& At(size_t ind) const {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on At const");
        }
        return std::get<Vec>(value_).at(ind);
    }

    template <typename T>
    std::vector<T>& GetVector() {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on GetVector");
        }
        return std::get<Vec>(value_);
    }

    template <typename T>
    const std::vector<T>& GetVector() const {
        using Vec = std::vector<T>;
        if (!std::holds_alternative<Vec>(value_)) {
            throw std::runtime_error("Column type mismatch on GetVector const");
        }
        return std::get<Vec>(value_);
    }

    ColumnValue& Value();
    const ColumnValue& Value() const;

private:
    Type type_;
    ColumnValue value_;
};
