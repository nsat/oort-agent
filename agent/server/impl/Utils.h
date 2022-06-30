/**
 * Utils.h
 *
 * Because every project needs one.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <semaphore.h>
#include <string>

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
class BinSemaphore final {
    sem_t sem;
 public:
    BinSemaphore();
    ~BinSemaphore();
    BinSemaphore(const BinSemaphore&) = delete;
    BinSemaphore& operator=(const BinSemaphore&) = delete;
    BinSemaphore(BinSemaphore&&) = delete;
    BinSemaphore& operator=(BinSemaphore&&) = delete;
    int wait();
    int timedwait(const timespec& abs_timeout);
    int post();
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
