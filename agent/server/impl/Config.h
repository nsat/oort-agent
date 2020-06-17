/**
 * AgentConfig.h
 * 
 * Configuration for Agent
 * 
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>

#include "Log.h"

/**
 * \brief Configuration for Agent.
 * 
 * Handles configuration option parsing and validation.
 * 
 */
class AgentConfig {
    friend class Agent;
    bool initialized = false;
    bool wantUsage = false;

    std::string workdir;  ///< Agent working directory
    int minFreeMb = -1;  ///< minimum free disk in MB
    int minFreePct = -1;  ///< minimum free disk percentage

    int cleanupInterval;  ///< Cleaner run interval
    int maxAge;  ///< Cleaner maximum file age

    int port;  ///< tcp port to listen on

    // options:
    // w - workdir
    // t - timeout
    // i - interval
    // f - config file
    // s - syslog identifier
    // m - min disk free
    // p - port
    // l - loglevel
    const char *optstring = "w:t:i:f:s:m:p:l:";

    struct {
        bool minfree = true;
        bool interval = true;
        bool maxage = true;
        bool port = true;
        bool loglevel = true;
    } use_defaults;

    const struct {
        int minfree = 20;  // default = 20%, not MB
        int interval = 3600;
        int maxage = 86400;
        int port = 2005;
        Log::levels loglevel = Log::levels::Warn;
    } defaults;

    void checkDir(const std::string &dir);

 public:
    bool parseOptions(int argc, char *const argv[]);
    void usage(const char *cmd);
    int getPort();
};
