/**
 * Log.h
 *
 * Simple logging class.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <syslog.h>
#include <string>
#include <vector>

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
    LogArg(const unsigned int val) :  // NOLINT(runtime/explicit)
        std::string(std::to_string(val)) {}
    LogArg(const int64_t val) : std::string(std::to_string(val)) {}  // NOLINT(runtime/explicit)
    LogArg(const double val) : std::string(std::to_string(val)) {}  // NOLINT(runtime/explicit)
};

namespace Log {
    const LogArg EMPTY("<>");
    extern thread_local std::string thread_name;
    enum levels {Debug, Info, Warn, Error};
    const std::vector<int> syslogLevels({LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERR});
    const std::vector<std::string> levelNames({"debug", "info", "warn", "error"});

#define LOG_DECL(LEVEL) \
    void LEVEL(const std::string &msg, \
        const LogArg &param1 = EMPTY, const LogArg &param2 = EMPTY, \
        const LogArg &param3 = EMPTY, const LogArg &param4 = EMPTY, \
        const LogArg &param5 = EMPTY, const LogArg &param6 = EMPTY \
        )

    LOG_DECL(debug);
    LOG_DECL(info);
    LOG_DECL(warn);
    LOG_DECL(error);
#undef LOG_DECL

    void setLevel(const levels l);
    void setOut(std::ostream &out);
    void setSyslog(const std::string &ident);
    void setThreadName(const std::string &name);
    void setThreadDesc();
    const std::string getThreadDesc();
}  // namespace Log
