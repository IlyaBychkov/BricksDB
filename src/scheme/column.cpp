#include "scheme/column.h"

Column::Column(Type type) : type_(type) {
    if (type_ == Type::int64) {
        value_ = ColumnValue(std::vector<int64_t>{});
    } else {
        value_ = ColumnValue(std::vector<std::string>{});
    }
}

Type Column::GetType() const {
    return type_;
}

size_t Column::GetSize() const {
    return std::visit([](auto&& v) -> size_t { return v.size(); }, value_);
}

Column::ColumnValue& Column::Value() {
    return value_;
}
const Column::ColumnValue& Column::Value() const {
    return value_;
}
