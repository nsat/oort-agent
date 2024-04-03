/**
 * Log.cpp
 *
 * Simple logging class.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include "Log.h"
#include "Utils.h"
#include <syslog.h>
#include <time.h>
#include <atomic>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace Log {
thread_local std::string thread_name = "main";
thread_local std::string thread_desc;
thread_local int thread_num = -1;
atomic_int thread_count{0};

namespace {
    // private functions/data
    levels l_level = Info;
    ostream *l_out = &cerr;
    const string l_timefmt  = "[%Y-%m-%d %H:%M:%SZ] ";
    bool using_syslog = false;
    string l_ident;
    BinSemaphore l_sem;

    string fmttime() {
        char time_buf[100];
        time_t t = time(NULL);
        if (strftime(time_buf, sizeof(time_buf), l_timefmt.c_str(), gmtime(&t)) != 0) {
            return time_buf;
        } else {
            return {};
        }
    }

    void log(const levels level, const string &msg, vector<LogArg> params) {
        if (level >= l_level) {
            stringstream s_out;

            if (!using_syslog) {
                s_out << fmttime();
            }

            s_out << levelNames[level] << " ";
            s_out << "[" << getThreadDesc() << "] ";

            auto mstart = 0, ix = 0;
            auto m = msg.find('?', mstart);
            while (m != string::npos && ix < params.size()) {
                s_out << msg.substr(mstart, m-mstart) << params[ix];
                mstart = m + 1;
                ++ix;
                m = msg.find('?', mstart);
            }
            s_out << msg.substr(mstart);

            l_sem.wait();
            if (using_syslog) {
                syslog(Log::syslogLevels[level], "%s", s_out.str().c_str());
            } else {
                *l_out << s_out.str() << endl;
            }
            l_sem.post();
        }
    }
}  // namespace

#define LOG_DEFN(LEVEL, LOGLEVEL) \
void LEVEL(const string &msg, \
        const LogArg &param1, const LogArg &param2, \
        const LogArg &param3, const LogArg &param4, \
        const LogArg &param5, const LogArg &param6 \
        ) { \
    log(LOGLEVEL, msg, vector<LogArg>{param1, param2, param3, param4, param5, param6}); \
}

LOG_DEFN(debug, Debug);
LOG_DEFN(info, Info);
LOG_DEFN(warn, Warn);
LOG_DEFN(error, Error);
#undef LOG_DEFN

void setLevel(const levels l) {
    l_level = l;
}

void setOut(ostream &out) {
    using_syslog = false;
    closelog();
    l_out = &out;
}

void setSyslog(const std::string &ident) {
    using_syslog = true;
    l_ident = string(ident);
    openlog(l_ident.c_str(), 0, LOG_USER);
}

void setThreadName(const std::string &name) {
    thread_name = name;
    setThreadDesc();
}

void setThreadDesc() {
    thread_desc = thread_name + ":" + to_string(thread_num);
}

const std::string getThreadDesc() {
    if (thread_num == -1) {
        thread_num = thread_count.fetch_add(1);
        setThreadDesc();
    }
    return thread_desc;
}

}  // namespace Log
