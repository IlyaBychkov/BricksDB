#pragma once

#include <expected>
#include <string>

enum class Type {
    int64 = 0,
    string = 1,
};

std::expected<Type, std::string> StringToType(const std::string& type_str);
std::string TypeToString(Type type);
