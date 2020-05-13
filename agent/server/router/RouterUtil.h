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
