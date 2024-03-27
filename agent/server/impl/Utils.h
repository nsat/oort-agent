/**
 * Utils.h
 *
 * Because every project needs one.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <semaphore.h>
#include <unistd.h>
#include <ext/stdio_filebuf.h>
#include <string>
#include <ostream>

#include "onion/onion.hpp"
#include "onion/response.hpp"
#include "ErrorResponse.h"

using namespace org::openapitools::server::model;

// wrapper around response code to ease changing to something else
enum class Code {
    Ok = HTTP_OK,
    Bad_Request = HTTP_BAD_REQUEST,
};

template <class T>
class ResponseCode {
 public:
    Code code;
    T result;
    ErrorResponse err;
};

template <class T>
std::string to_jsonstr(T obj) {
    nlohmann::json j;
    to_json(j, obj);
    return j.dump();
}

template <class F, class T>
void from_jsonfile(F &stream, T &obj) {
    nlohmann::json j = nlohmann::json::parse(stream);
    from_json(j, obj);
}

template <class T>
onion_connection_status deliverResponse(Onion::Response &rstream, ResponseCode<T> &response) {
    switch (response.code) {
        case Code::Ok :
            rstream << to_jsonstr(response.result);
            break;
        default:
            response.err.setStatus(static_cast<int>(response.code));
            rstream.setCode(static_cast<int>(response.code));
            rstream << to_jsonstr(response.err);
            break;
    }
    return OCS_PROCESSED;
}

// simple semaphore wrapper
class Semaphore {
    sem_t sem;
 public:
    explicit Semaphore(int n);
    ~Semaphore();
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    Semaphore& operator=(Semaphore&&) = delete;
    int wait();
    int trywait();
    int timedwait(const timespec& abs_timeout);
    int post();
};

// Binary semaphore version of simple wrapper.
class BinSemaphore : public Semaphore {
 public:
    BinSemaphore() : Semaphore(1) {}
};

/**
 * @brief RAII semaphore guard wrapper, similar to lock_guard.
 * Provides operations to wait on a semaphore and automatically
 * post it when the guard is destroyed.
 */
class sem_guard final {
    Semaphore *m_sem;
    bool m_owned = false;
 public:
    explicit sem_guard(Semaphore &sem);
    ~sem_guard();
    bool wait();
    bool trywait();
};

/**
 * @brief filebuf variant that fsyncs in addition to flushing buffers
 * and syncs on destruction.
 *
 * Use the gcc extension stdio_filebuf to get access
 * to the underlying fd so we can fsync it.
 * fsync is required so that the meta file doesn't end up
 * empty when there is a hard shutdown soon after writing.
 * https://jira.spire.com/browse/CPL-130
 */
class sync_ofilebuf : public __gnu_cxx::stdio_filebuf<char> {
 protected:
    virtual int sync();
 public:
    ~sync_ofilebuf();
};

/**
 * @brief ofstream variant that uses a \ref sync_ofilebuf to ensure
 * data is fsynced to disk when closing.
 *
 */
class sync_ofstream : public std::ofstream {
 protected:
    sync_ofilebuf fb;
 public:
    explicit sync_ofstream(const std::string& filename);
    void open(const std::string& filename);
};

std::string mkUUID();
std::string OSError(const std::string &prefix = "");
std::string getCrc(const std::string &file);
std::string tailname(const std::string &filepath);
std::string dirname(const std::string &filepath);
std::string rootname(const std::string &filepath);
std::string extension(const std::string &filepath);
bool ends_with(const std::string &str, const std::string &tail);
std::string chop(const std::string &str, const std::string &tail);
bool starts_with(const std::string &str, const std::string &head);
std::string behead(const std::string &str, const std::string &head);
int runProcessGetOutput(const std::string &cmd, const char *const argv[],
                        size_t max_output_size, uint32_t timeout_seconds, std::string &output);
