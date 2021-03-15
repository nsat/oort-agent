/**
 * SdkApiRouter.h
 *
 * OORT Agent SDK router
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>

#include "onion/url.hpp"
#include "onion/response.hpp"
#include "RetrieveFileRequest.h"
#include "SendFileRequest.h"

using org::openapitools::server::model::RetrieveFileRequest;
using org::openapitools::server::model::SendFileRequest;

class SdkApiRouter : public Onion::Url {
 public:
    SdkApiRouter();

    virtual void query_available_files(
        const std::string &topic, Onion::Response &response) = 0;
    virtual void retrieve_file(
        const RetrieveFileRequest &retrieveFileRequest, Onion::Response &response) = 0;
    virtual void send_file(
        const SendFileRequest &sendFileRequest, Onion::Response &response) = 0;
};