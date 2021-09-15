/**
 * SdkApiRouter.cpp
 *
 * OORT Agent SDK router
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include <string>

#include "SdkApiRouter.h"
#include "onion/handler.hpp"
#include "onion/request.hpp"
#include "onion/url.hpp"
#include "onion/block.h"
#include "nlohmann/json.hpp"
#include "Log.h"

#include "RouterUtil.h"

using namespace std;

SdkApiRouter::SdkApiRouter() {
    this->add("^v1/query_available_files/([-_[:alnum:]]+)$",
     [this](Onion::Request &req, Onion::Response &resp) {
        CheckMethod(req, resp, GET);
        string topic = req.query("1");
        this->query_available_files(topic, resp);
        return OCS_PROCESSED;
    });
    this->add("^v1/send_file$", [this](Onion::Request &req, Onion::Response &resp) {
        CheckMethod(req, resp, POST);
        SendFileRequest sreq;
        GetPostData(sreq, req);
        this->send_file(sreq, resp);
        return OCS_PROCESSED;
    });
    this->add("^v1/retrieve_file$", [this](Onion::Request &req, Onion::Response &resp) {
        CheckMethod(req, resp, POST);
        RetrieveFileRequest rreq;
        GetPostData(rreq, req);
        this->retrieve_file(rreq, resp);
        return OCS_PROCESSED;
    });
    this->add("^v1/adcs$", [this](Onion::Request &req, Onion::Response &resp) {
        if (GetMethod(req) == OR_GET) {
            this->adcs_get(resp);
        } else if (GetMethod(req) == OR_POST) {
            AdcsSetRequest areq;
            GetPostData(areq, req);
            this->adcs_post(areq, resp);
        } else {
            BadMethod(resp);
        }
        return OCS_PROCESSED;
    });
    this->add("^.*", "Method not implemented", HTTP_NOT_IMPLEMENTED);
}

