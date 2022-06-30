/**
 * main-api-server.cpp
 *
 * OORT Agent main()
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */


#include "CollectorApiRouter.h"
#include <string>

#include "onion/handler.hpp"
#include "onion/url.hpp"
#include "onion/request.hpp"
#include "onion/response.hpp"

#include "InfoRequest.h"
#include "Log.h"
#include "RouterUtil.h"
#include "Utils.h"

using namespace std;

CollectorApiRouter::CollectorApiRouter() {
    Log::info("setting up collector routes");
    this->add("v1/ping", [this](Onion::Request &req, Onion::Response &resp) {
        InfoRequest ireq;
        CheckMethod(req, resp, GET);
        this->ping(resp);
        return OCS_PROCESSED;
    });
    this->add("v1/info", [this](Onion::Request &req, Onion::Response &resp) {
        InfoRequest ireq;
        CheckMethod(req, resp, POST);
        GetPostData(ireq, req);
        this->info(ireq, resp);
        return OCS_PROCESSED;
    });
    this->add("^v1/meta/([-[:xdigit:]]+)$",
     [this](Onion::Request &req, Onion::Response &resp) {
        CheckMethod(req, resp, GET);
        string uuid = req.query("1");
        this->meta(uuid, resp);
        return OCS_PROCESSED;
    });
    this->add("^.*", "Method not implemented", HTTP_NOT_IMPLEMENTED);
}
