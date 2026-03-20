#pragma once

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "type.h"

struct Column {
public:
    using ColumnValue = std::variant<std::vector<int64_t>, std::vector<int32_t>,
                                     std::vector<int16_t>, std::vector<std::string>>;

    Column(Type type);
    Column(Type type, int64_t size);

    Type GetType() const;
    size_t GetSize() const;

    template <typename T>
    void Push(const T& v) {
        if (!std::holds_alternative<std::vector<T>>(value_)) {
            throw std::runtime_error("Column type mismatch on Push");
        }
        std::get<std::vector<T>>(value_).push_back(v);
    }

    template <typename T>
    const T& GetValue(size_t ind) const {
        if (!std::holds_alternative<std::vector<T>>(value_)) {
            throw std::runtime_error("Column type mismatch on GetValue");
        }
        return std::get<std::vector<T>>(value_)[ind];
    }

    template <typename T>
    const std::vector<T>& GetVector() const {
        if (!std::holds_alternative<std::vector<T>>(value_)) {
            throw std::runtime_error("Column type mismatch on GetVector");
        }
        return std::get<std::vector<T>>(value_);
    }

    ColumnValue& Value();
    const ColumnValue& Value() const;

private:
    Type type_;
    ColumnValue value_;
};

std::expected<void, std::string> WriteColumnToColumnar(const Column& column, std::ofstream& fout);
std::expected<Column, std::string> ReadColumnFromColumnar(Type type, std::ifstream& fin,
                                                          int64_t rows_cnt);
