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
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace Log {
thread_local std::string thread_name = "main";

namespace {
    // private functions/data
    levels l_level = Info;
    ostream *l_out = &cerr;
    const char *DEFAULT_TIMEFMT  = "[%Y-%m-%d %H:%M:%SZ] ";
    string *l_timefmt = NULL;
    bool using_syslog = false;
    string l_ident;
    BinSemaphore l_sem;

    string fmttime() {
        if (!l_timefmt) {
            l_timefmt = new string(DEFAULT_TIMEFMT);
        }
        char time_buf[100];
        time_t t = time(NULL);
        if (strftime(time_buf, sizeof(time_buf), l_timefmt->c_str(), gmtime(&t)) != 0) {
            return time_buf;
        } else {
            return {};
        }
    }

    void log(const levels level, const string &msg, const string &param1, const string &param2) {
        if (level >= l_level) {
            stringstream s_out;

            if (!using_syslog) {
                s_out << fmttime();
            }

            s_out << levelNames[level] << " ";
            s_out << "[" << thread_name << "] ";

            auto mstart = 0;
            auto m = msg.find('?', mstart);
            if (m != string::npos) {
                s_out << msg.substr(mstart, m) << param1;
                mstart = m + 1;
            }
            m = msg.find('?', mstart);
            if (m != string::npos) {
                s_out << msg.substr(mstart, m-mstart) << param2;
                mstart = m + 1;
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

void debug(const string &msg, const LogArg &param1, const LogArg &param2) {
    log(Debug, msg, param1, param2);
}

void info(const string &msg, const LogArg &param1, const LogArg &param2) {
    log(Info, msg, param1, param2);
}

void warn(const string &msg, const LogArg &param1, const LogArg &param2) {
    log(Warn, msg, param1, param2);
}

void error(const string &msg, const LogArg &param1, const LogArg &param2) {
    log(Error, msg, param1, param2);
}

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
}

}  // namespace Log
