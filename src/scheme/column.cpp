#include "scheme/column.h"

#include <fstream>

#include "scheme/overloaded.h"

Column::Column(Type type) : type_(type) {
    if (type_ == Type::int64) {
        value_ = ColumnValue(std::vector<int64_t>{});
    } else {
        value_ = ColumnValue(std::vector<std::string>{});
    }
}

Column::Column(Type type, int64_t size) : type_(type) {
    if (type_ == Type::int64) {
        value_ = ColumnValue(std::vector<int64_t>(size));
    } else {
        value_ = ColumnValue(std::vector<std::string>(size));
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

std::expected<void, std::string> WriteColumnToColumnar(const Column& column, std::ofstream& fout) {
    auto visitor = Overloaded{
        [&fout](const std::vector<int64_t>& vec) {
            fout.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(int64_t));
        },
        [&fout](const std::vector<std::string>& vec) {
            std::vector<int64_t> lens;
            for (const auto& str : vec) {
                lens.push_back(static_cast<int64_t>(str.size()));
            }

            fout.write(reinterpret_cast<const char*>(lens.data()), lens.size() * sizeof(int64_t));
            for (const auto& str : vec) {
                fout.write(str.data(), str.size());
            }
        }};

    try {
        std::visit(visitor, column.Value());
    } catch (...) {
        return std::unexpected("Failed to write column to columnar file");
    }
    return {};
}

std::expected<Column, std::string> ReadColumnFromColumnar(Type type, std::ifstream& fin,
                                                          int64_t rows_cnt) {
    Column column(type, rows_cnt);

    auto visitor = Overloaded{
        [&fin, rows_cnt](std::vector<int64_t>& vec) {
            vec.resize(rows_cnt);
            fin.read(reinterpret_cast<char*>(vec.data()), vec.size() * sizeof(int64_t));
        },
        [&fin, rows_cnt](std::vector<std::string>& vec) {
            std::vector<int64_t> lens(rows_cnt);
            fin.read(reinterpret_cast<char*>(lens.data()), lens.size() * sizeof(int64_t));

            vec.resize(rows_cnt);
            for (int64_t i = 0; i < rows_cnt; ++i) {
                vec[i].resize(lens[i]);
                fin.read(reinterpret_cast<char*>(vec[i].data()), lens[i]);
            }
        }};

    try {
        std::visit(visitor, column.Value());
    } catch (...) {
        return std::unexpected("Failed to read column from columnar file");
    }
    return column;
}
