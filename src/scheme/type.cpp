#include "type.h"

#include <stdexcept>
#include <string>

std::expected<Type, std::string> StringToType(const std::string& type_str) {
    if (type_str == "int64") {
        return Type::int64;
    } else if (type_str == "string") {
        return Type::string;
    }
    return std::unexpected("Unknown type: " + type_str);
}

std::string TypeToString(Type type) {
    switch (type) {
        case Type::int64:
            return "int64";
        case Type::string:
            return "string";
        default:
            throw std::runtime_error("Unknown Type enum value");
    }
}
