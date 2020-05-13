/**
 * AgentConfig.cpp
 * 
 * Configuration for Agent
 * 
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include "Config.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

#include "Files.h"
#include "Log.h"

using namespace std;

/**
 * \brief checks that the directory meets the agent needs
 * 
 * Throws `invalid_argument` if any issues are found.
 * 
 */
void AgentConfig::checkDir(const string &dir) {
    struct stat buf;

    if (!Files::checkPath(dir)) {
        throw invalid_argument("directory not absolute");
    } else if (stat(dir.c_str(), &buf) != 0) {
        throw invalid_argument("directory not found");
    } else if (!S_ISDIR(buf.st_mode)) {
        throw invalid_argument("not a directory");
    } else if (buf.st_uid != geteuid()) {
        throw invalid_argument("directory not owned by current user");
    } else if ((buf.st_mode & S_IRWXU) != S_IRWXU) {
        throw invalid_argument("incorrect permissions on directory (must be u=rwx)");
    }
}

/**
 * \brief parses command line options
 * 
 * \return true there were no errors parsing options
 * \return false there was an error parsing options
 */
bool AgentConfig::parseOptions(int argc, char *const argv[]) {
    int opt;
    optind = 1;
    opterr = 0;
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'w':
                try {
                    workdir = optarg;
                    checkDir(workdir);
                }
                catch (const invalid_argument& e) {
                    cerr << "Invalid directory " << optarg << ": " << e.what() << endl;
                    return false;
                }
                break;
            case 't':
                try {
                    maxAge = stoul(optarg);
                }
                catch (const invalid_argument& e) {
                    cerr << "Invalid age " << optarg << endl;
                    return false;
                }
                use_defaults.maxage = false;
                break;
            case 'i':
                try {
                    cleanupInterval = stoul(optarg);
                }
                catch (const invalid_argument& e) {
                    cerr << "Invalid timeout " << optarg << endl;
                    return false;
                }
                use_defaults.interval = false;
                break;
            case 'f':
                cerr << "Config file processing is not yet implemented" << endl;
                return false;
            case 's':
                Log::setSyslog(optarg);
                break;
            case 'm':
                try {
                    size_t pos;
                    int min_num = stoul(optarg, &pos);
                    if (*(optarg + pos) == 'M') {
                        minFreeMb = min_num;
                    } else if (*(optarg + pos) == '%') {
                        minFreePct = min_num;
                    } else {
                        throw invalid_argument("no recognized unit");
                    }
                }
                catch (const invalid_argument& e) {
                    cerr << "Unsupported min_free setting '" << optarg << "'" << endl;
                    return false;
                }
                use_defaults.minfree = false;
                break;
            case 'p':
                try {
                    port = stoul(optarg);
                }
                catch (const invalid_argument& e) {
                    cerr << "Invalid port " << optarg << endl;
                    return false;
                }
                use_defaults.port = false;
                break;
            case '?':
                // missing argument
                wantUsage = true;
                return false;
            default:
                cerr << "Unknown option '" << opt << "'" << endl;
                return false;
        }
    }

    if (workdir.empty()) {
        cerr << "Workdir must be specified" << endl;
        return false;
    }

    // apply defaults if needed
    if (use_defaults.interval) {
       cleanupInterval = defaults.interval;
    }
    if (use_defaults.maxage) {
        maxAge = defaults.maxage;
    }
    if (use_defaults.minfree) {
        minFreePct = defaults.minfree;
    }
    if (use_defaults.port) {
        port = defaults.port;
    }

    initialized = true;
    return true;
}

void AgentConfig::usage(const char *cmd) {
    if (!wantUsage) return;
    cerr << "Usage:" << endl;
    cerr << cmd << " -w workdir" << endl;
    cerr << " [-t cleanup-timeout] [-i cleanup-interval] [-f config-file]" << endl;
    cerr << " [-s ident] [-m minfree] [-p port]" << endl;
    cerr << " workdir - base working directory; must be writable" << endl;
    cerr << " cleanup-timeout - age in seconds after which files can be deleted" << endl;
    cerr << " cleanup-interval - how frequently in seconds to run the cleanup task" << endl;
    cerr << " config-file - file to use for setting config values (not implemented)" << endl;
    cerr << " ident - identifier for syslog (default is to log to stderr)" << endl;
    cerr << " minfree - minimum free space to maintain; either ddM or dd%" << endl;
    cerr << " port - tcp port to listen on" << endl;
    cerr << endl;
    cerr << "Defaults: " << endl;
    cerr << " cleanup-timeout = " << defaults.maxage;
    cerr << "  cleanup-interval = " << defaults.interval << endl;
    cerr << " minfree = " << defaults.minfree << "%" << endl;
    cerr << " port = " << defaults.port << endl;
}

int AgentConfig::getPort() {
    return port;
}
