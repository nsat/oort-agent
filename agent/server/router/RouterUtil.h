/**
 * RouterUtil.h
 *
 * OORT Agent utilities for router methods
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include "nlohmann/json.hpp"

#define GetMethod(REQ) (REQ.flags() & OR_METHODS)

// wrapper around request parsing that will return a 400 error
// for an error, whether it's invalid json or missing keys
#define ParseRequest(PARSED_REQ, ONION_REQ, RESP) try {       \
        GetPostData(PARSED_REQ, ONION_REQ);                   \
    } catch (const nlohmann::json::exception &ex) {           \
        Log::warn("Error parsing request: ?", ex.what());     \
        RESP.setCode(HTTP_BAD_REQUEST);                       \
        RESP << "Bad request: " << ex.what() << std::endl ;   \
        return OCS_PROCESSED;                                 \
    }

#define BadMethod(RESP) do {                \
    RESP.setCode(HTTP_METHOD_NOT_ALLOWED);  \
    RESP << "Method not allowed";           \
    } while (0)

#define CheckMethod(REQ, RESP, METHOD) do {    \
    if (GetMethod(REQ) != OR_ ## METHOD) {     \
        BadMethod(RESP);                       \
        return OCS_PROCESSED;                  \
    }} while (0)


template<class T>
void GetPostData(T &var, Onion::Request &req) {
    const onion_block *reqd = onion_request_get_data(req.c_handler());
    if (!reqd) {
        Log::warn("no data!");
        return;
    }
    const char *data = onion_block_data(reqd);
    nlohmann::json req_json = nlohmann::json::parse(data);
    from_json(req_json, var);
}
