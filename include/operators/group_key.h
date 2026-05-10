#pragma once

#include <string>
#include <variant>
#include <vector>

using ValueType = std::variant<int64_t, int32_t, int16_t, std::string>;
using GroupKey = std::vector<ValueType>;

struct GroupKeyHash {
    std::size_t operator()(const GroupKey& key) const {
        std::size_t seed = 0;
        for (const auto& val : key) {
            std::visit(
                [&seed]<typename T>(const T& val) {
                    std::size_t h = std::hash<T>{}(val);
                    seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                },
                val);
        }
        return seed;
    }
};