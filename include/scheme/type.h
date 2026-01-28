#pragma once

#include <expected>
#include <string>

enum class Type {
    int64,
    string,
};

std::expected<Type, std::string> StringToType(const std::string& type_str);
std::string TypeToString(Type type);
