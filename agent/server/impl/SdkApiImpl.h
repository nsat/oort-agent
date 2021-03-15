/**
 * SdkApiImpl.h
 *
 * OORT Agent SDK Api interface.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>
#include <memory>

#include <SdkApiRouter.h>

#include "ErrorResponse.h"
#include "FileInfo.h"
#include "RetrieveFileRequest.h"
#include "AvailableFilesResponse.h"
#include "SendFileRequest.h"
#include "SendFileResponse.h"

#include "Agent.h"
#include "Utils.h"

using org::openapitools::server::model::RetrieveFileRequest;
using org::openapitools::server::model::SendFileRequest;

class SdkApiImpl : public SdkApiRouter {
 public:
    explicit SdkApiImpl(Agent *agent);

    void query_available_files(
        const std::string &topic, Onion::Response &response);
    void retrieve_file(
        const RetrieveFileRequest &retrieveFileRequest, Onion::Response &response);
    void send_file(
        const SendFileRequest &sendFileRequest, Onion::Response &response);

 private:
    Agent *m_agent;
};
