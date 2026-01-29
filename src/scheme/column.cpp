#include "scheme/column.h"

#include <stdexcept>

Column::Column(Type type) : type_(type) {
    if (type == Type::int64) {
        value_ = ColumnValue(std::vector<int64_t>{});
    } else if (type == Type::string) {
        value_ = ColumnValue(std::vector<std::string>{});
    } else {
        throw std::runtime_error("Unsupported type in Column");
    }
}

Type Column::GetType() const {
    return type_;
}

size_t Column::GetSize() const {
    return std::visit([](auto&& v) -> size_t { return v.size(); }, value_);
}

template <typename T>
void Column::Push(const T& v) {
    using Vec = std::vector<T>;
    if (!std::holds_alternative<Vec>(value_)) {
        throw std::runtime_error("Mismatch type in Column Push");
    }
    std::get<Vec>(value_).push_back(v);
}

template <typename T>
T& Column::At(size_t ind) {
    using Vec = std::vector<T>;
    if (!std::holds_alternative<Vec>(value_)) {
        throw std::runtime_error("Mismatch type in Column At");
    }
    return std::get<Vec>(value_).at(ind);
}

template <typename T>
const T& Column::At(size_t ind) const {
    using Vec = std::vector<T>;
    if (!std::holds_alternative<Vec>(value_)) {
        throw std::runtime_error("Mismatch type in Column At const");
    }
    return std::get<Vec>(value_).at(ind);
}

template <typename T>
std::vector<T>& Column::GetVector() {
    using Vec = std::vector<T>;
    if (!std::holds_alternative<Vec>(value_)) {
        throw std::runtime_error("Mismatch type in Column GetVector");
    }
    return std::get<Vec>(value_);
}

template <typename T>
const std::vector<T>& Column::GetVector() const {
    using Vec = std::vector<T>;
    if (!std::holds_alternative<Vec>(value_)) {
        throw std::runtime_error("Mismatch type in Column GetVector const");
    }
    return std::get<Vec>(value_);
}

Column::ColumnValue& Column::Value() {
    return value_;
}
const Column::ColumnValue& Column::Value() const {
    return value_;
}
