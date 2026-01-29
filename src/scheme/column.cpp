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

Type Column::GetType() const {
    return type_;
}

size_t Column::GetSize() const {
    return std::visit([](auto&& v) -> size_t { return v.size(); }, value_);
}

Column::ColumnValue& Column::Value() {
    return value_;
}
bool Column::WriteToFile(std::ofstream& fout) const {
    auto visitor = Overloaded{[&fout](const std::vector<int64_t>& vec) {
                                  fout.write(reinterpret_cast<const char*>(vec.data()),
                                             vec.size() * sizeof(int64_t));
                              },
                              [&fout](const std::vector<std::string>& vec) {
                                  int64_t pos = 0;
                                  std::vector<int64_t> endings;

                                  for (const auto& str : vec) {
                                      pos += static_cast<int64_t>(str.size());
                                      endings.push_back(pos);
                                  }

                                  fout.write(reinterpret_cast<const char*>(endings.data()),
                                             endings.size() * sizeof(int64_t));

                                  for (const auto& str : vec) {
                                      fout.write(str.data(), str.size());
                                  }
                              }};

    std::visit(visitor, value_);
    return true;
}