#include "scheme/type.h"

#include <stdexcept>
#include <string>

std::expected<Type, std::string> StringToType(const std::string& type_str) {
    if (type_str == "int64") {
        return Type::int64;
    } else if (type_str == "string") {
        return Type::string;
    } else if (type_str == "int16") {
        return Type::int16;
    } else if (type_str == "int32") {
        return Type::int32;
    }
    return std::unexpected("StringToType: Unknown type: " + type_str);
}

std::string TypeToString(Type type) {
    switch (type) {
        case Type::int64:
            return "int64";
        case Type::string:
            return "string";
        case Type::int16:
            return "int16";
        case Type::int32:
            return "int32";
        default:
            throw std::runtime_error("Unknown Type enum value");
    }
}
