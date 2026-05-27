#pragma once

#include <cstdint>
#include <expected>
#include <string>

std::expected<int64_t, std::string> TimestampToInt(std::string s);

std::expected<int32_t, std::string> DateToInt(std::string s);

std::string IntToTimestamp(int64_t t);

std::string IntToDate(int32_t d);

int64_t ExtractMinute(int64_t ts);
