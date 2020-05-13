/**
 * Log.h
 *
 * Simple logging class.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>

/**
 * \brief LogArg - string with implicit conversions
 * 
 * This class is a simple subclass of std::string, with the
 * addition of an implicit conversion from int to string.  
 * This is used to simplify the logging interface, so logging
 * an int value doesn't need to be wrapped in a conversion.
 */
class LogArg : public std::string {
 public:
    LogArg(const std::string &s) : std::string(s) {}  // NOLINT(runtime/explicit)
    LogArg(const char *s) : std::string(s) {}  // NOLINT(runtime/explicit)
    LogArg(const int val) : std::string(std::to_string(val)) {}  // NOLINT(runtime/explicit)
    LogArg(const int64_t val) : std::string(std::to_string(val)) {}  // NOLINT(runtime/explicit)
};

namespace Log {
    const LogArg EMPTY("<>");
    extern thread_local std::string thread_name;
    enum levels {Debug, Info, Warn, Error};

    void debug(const std::string &msg,
        const LogArg &param1 = EMPTY, const LogArg &param2 = EMPTY);
    void info(const std::string &msg,
        const LogArg &param1 = EMPTY, const LogArg &param2 = EMPTY);
    void warn(const std::string &msg,
        const LogArg &param1 = EMPTY, const LogArg &param2 = EMPTY);
    void error(const std::string &msg,
        const LogArg &param1 = EMPTY, const LogArg &param2 = EMPTY);
    void setLevel(const levels l);
    void setOut(std::ostream &out);
    void setSyslog(const std::string &ident);
    void setThreadName(const std::string &name);
}  // namespace Log
