/*
 * Utils.cpp
 * 
 * Utility functions.
 * 
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include <zlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>  // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "Log.h"
#include "Utils.h"

#define PROCESS_OUTPUT_CHUNK_SIZE 4096
#define RUN_ERROR_STATUS 127

using namespace std;

string mkUUID() {
    char rand_dat[16];

    ifstream urand("/dev/urandom");
    urand.read(rand_dat, sizeof(rand_dat));
    urand.close();

    rand_dat[6] = 0x40 | (rand_dat[6] & 0x0F);
    rand_dat[8] = 0x80 | (rand_dat[8] & 0x3F);

    stringstream uuid;
    uuid << hex << setfill('0');
    int i = 0;
    // time-low
    for (int c = 0; c < 4; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // time-mid
    for (int c = 0; c < 2; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // time-high-and-version
    for (int c = 0; c < 2; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // clock-seq-and-reserved
    for (int c = 0; c < 1; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    // clock-seq-low
    for (int c = 0; c < 1; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // node
    for (int c = 0; c < 6; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    return uuid.str();
}

string OSError(const string &prefix) {
    stringstream errmsg;
    errmsg << prefix;
    char buf[100];
    // Annoyingly, gnu libc uses the same function prototype for strerror_r,
    // but completely different behavior from POSIX.
    // strerror_r(errno, buf, 100);
    // errmsg << buf;
    errmsg << strerror_r(errno, buf, sizeof(buf));
    return errmsg.str();
}

string getCrc(const string &file) {
    constexpr int kBlocksize = 1024;
    char c[kBlocksize];
    ulong crc = crc32(0L, 0, 0);
    ifstream ifs(file);
    do {
        ifs.read(c, kBlocksize);
        crc = crc32(crc, (unsigned char*)c, ifs.gcount());
    } while (ifs.rdstate() == ios::goodbit);

    stringstream crcx;
    crcx << hex << setfill('0') << setw(8) << crc;
    return crcx.str();
}

/**
 * return the portion of the file path after the last "/"
 */
string tailname(const string &filepath) {
    auto basedir = filepath.rfind("/");
    if (basedir == string::npos) {
        return filepath;
    } else {
        return filepath.substr(basedir + 1);
    }
}

/**
 * return the portion of the file path before the last "/"
 */
string dirname(const string &filepath) {
    auto basedir = filepath.rfind("/");
    if (basedir == string::npos) {
        return filepath;
    } else {
        return filepath.substr(0, basedir);
    }
}

/**
 * return the portion of the file name before the last "."
 * 
 * Note that this is strictly textual, so the root of 
 * "foo.tar.gz" is "foo.tar" not "foo"
 */
string rootname(const string &filepath) {
    auto ext = filepath.rfind(".");
    if (ext == string::npos) {
        return filepath;
    } else {
        return filepath.substr(0, ext);
    }
}

/**
 * return the portion of the file name after the last ".", including the "."
 * If no extension, return empty string.
 */
string extension(const string &filepath) {
    auto ext = filepath.rfind(".");
    if (ext == string::npos) {
        return "";
    } else {
        return filepath.substr(ext);
    }
}

/**
 * checks if the given string ends with the tail. 
 * 
 */
bool ends_with(const string &str, const string &tail) {
    return (str.length() >= tail.length()) &&
        (str.substr(str.length() - tail.length()) == tail);
}

/**
 * checks if the given string starts with the head. 
 * 
 */
bool starts_with(const string &str, const string &head) {
    return (str.length() >= head.length()) &&
        (str.substr(0, head.length()) == head);
}


/**
 * removes a given tail from a string
 * If the string does not end with the tail, returns
 * the original string.
 * 
 */
string chop(const string &str, const string &tail) {
    auto s_tail = str.substr(str.length() - tail.length());
    if (s_tail != tail) {
        return str;
    } else {
        return str.substr(0, str.length() - tail.length());
    }
}

/**
 * removes a given head from a string
 * If the string does not start with the head, returns
 * the original string.
 * 
 */
string behead(const string &str, const string &head) {
    auto s_head = str.substr(0, head.length());
    if (s_head != head) {
        return str;
    } else {
        return str.substr(head.length());
    }
}

static bool read_into_string(int fd, char chunk[], size_t chunk_size,
                             size_t max_output_size, string &output) {
    ssize_t nbytes;
    do {
        nbytes = read(fd, chunk, chunk_size);
        if (nbytes < 0 || output.length() + static_cast<size_t>(nbytes) > max_output_size) {
            return false;
        }
        output.append(chunk, nbytes);
    } while (nbytes > 0);
    return true;
}

/**
 * Runs a child process and returns its stdout output
 * The child will be killed if it doesn't exit after timeout seconds, or if it
 * outputs more than max_output_size bytes, in which case the output string is not guaranteed
 * to hold the complete output.
 *
 * Returns the process exit code.
 *
 */
int runProcessGetOutput(const string &cmd, const char *const argv[], size_t max_output_size,
                        uint32_t timeout_seconds, string &output) {
    int link[2];
    pid_t pid;
    char chunk[PROCESS_OUTPUT_CHUNK_SIZE];
    sigset_t mask;
    sigset_t orig_mask;
    struct timespec timeout;
    size_t total_output_size = 0;

    // The pipe / fork / exec combo was adapted from https://stackoverflow.com/a/7292659
    // We could just use system() but this should be a bit less insecure.
    // We also adapt the sigtimedwait example from https://stackoverflow.com/a/20173592 but doing
    // the sigtimedwait in a separate thread.

    timeout.tv_sec = timeout_seconds;
    timeout.tv_nsec = 0;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    // NB: in normal operation we are in a thread; however in unit test we
    // are in the main process.  Using sigprocmask here is "unspecified" if
    // we are in a thread, but if the main thread has its mask set correctly
    // then we inherit it here.
    // Thus, for the signal handling to work properly SIGCHLD *must* be
    // blocked at startup in the main thread.
    if (pthread_sigmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
        throw system_error(errno, generic_category(), "pthread_sigmask");
    }

    if (pipe(link) == -1) {
        throw system_error(errno, generic_category(), "creating pipe");
    }

    pid = fork();
    if (pid == -1) {
        throw system_error(errno, generic_category(), "forking");
    } else if (pid == 0) {
        // Child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        try {
            execv(cmd.c_str(), (char* const*) argv);
        }
        catch (...) {
            Log::error("Unable to execv ?", cmd);
            exit(RUN_ERROR_STATUS);
        }
        Log::error("Unable to execv ?", cmd);
        exit(RUN_ERROR_STATUS);
    } else {
        // Parent
        close(link[1]);
        thread timeout_signal([&]() {
            do {
                if (sigtimedwait(&mask, NULL, &timeout) < 0) {
                    if (errno == EINTR) {
                        // Interrupted by a signal other than SIGCHLD.
                        continue;
                    } else if (errno == EAGAIN) {
                        Log::info("Child process timeout");
                        kill(pid, SIGKILL);
                        continue;
                    } else {
                        throw system_error(errno, generic_category(), "sigtimedwait");
                    }
                } else {
                    break;
                }
            } while (1);
        });

        int status;
        if (!read_into_string(link[0], chunk, sizeof(chunk),
                             max_output_size, output)) {
            kill(pid, SIGKILL);
        }

        timeout_signal.join();
        waitpid(pid, &status, 0);

        Log::debug("Child process output: ?", output);
        Log::debug("Child process status ?", status);
        close(link[0]);

        if (WIFSIGNALED(status)) {
            status = 128 + WTERMSIG(status);
        } else {
            status = WEXITSTATUS(status);
        }
        return status;
    }
}

BinSemaphore::BinSemaphore() {
    if (sem_init(&sem, 0, 1) != 0) {
        throw system_error(errno, generic_category(), "initializing semaphore");
    }
}

BinSemaphore::~BinSemaphore() {
    sem_destroy(&sem);
}

int BinSemaphore::wait() {
    return sem_wait(&sem);
}

int BinSemaphore::timedwait(const timespec& abs_timeout) {
    return sem_timedwait(&sem, &abs_timeout);
}

int BinSemaphore::post() {
    return sem_post(&sem);
}
