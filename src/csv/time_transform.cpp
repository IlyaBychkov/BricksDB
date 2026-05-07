#include "time_transform.h"

#include <chrono>
#include <cstdio>

std::expected<int64_t, std::string> TimestampToInt(std::string s) {
    int y, m, d, hh, mm, ss;
    if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) != 6) {
        return std::unexpected("TimestampToInt: Failed to parse timestamp: " + s);
    }
    auto ymd = std::chrono::year(y) / m / d;
    if (!ymd.ok()) {
        return std::unexpected("TimestampToInt: Invalid date in timestamp: " + s);
    }
    std::chrono::sys_seconds ts = std::chrono::sys_days{ymd} + std::chrono::hours(hh) +
                                  std::chrono::minutes(mm) + std::chrono::seconds(ss);
    return static_cast<int64_t>(ts.time_since_epoch().count());
}

std::expected<int32_t, std::string> DateToInt(std::string s) {
    int y, m, d;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) != 3) {
        return std::unexpected("DateToInt: Failed to parse date: " + s);
    }
    auto ymd = std::chrono::year(y) / m / d;
    if (!ymd.ok()) {
        return std::unexpected("DateToInt: Invalid date: " + s);
    }
    auto dt = std::chrono::sys_days{ymd};
    return static_cast<int32_t>(dt.time_since_epoch().count());
}

std::string IntToTimestamp(int64_t t) {
    std::chrono::sys_seconds ts{std::chrono::seconds{t}};
    return std::format("{:%F %T}", ts);
}

std::string IntToDate(int32_t d) {
    std::chrono::sys_days dt{std::chrono::days{d}};
    return std::format("{:%F}", dt);
}
