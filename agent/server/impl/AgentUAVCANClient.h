/**
 * AgentUAVCANClient.h
 *
 * Agent UAVCAN client task.  
 *
 * Copyright (c) 2021 Spire Global, Inc.
 */
#pragma once

#include <string>

#include <uavcan_linux/uavcan_linux.hpp>
#include <ussp/payload/PayloadAdcsCommand.hpp>
#include "Config.h"
#include "AdcsResponse.h"
#include "AdcsCommandRequest.h"
#include "AdcsCommandResponse.h"

static const unsigned UAVCLIENT_TIMEOUT = 1000;

/**
 * \brief Agent UAVCAN client
 * 
 * The UAVCAN client makes calls over UAVCAN to other services (generally
 * SBrain) and allows those services to be called from Agent handlers.
 * 
 */
class AgentUAVCANClient {
    AgentConfig& config;
    uavcan_linux::NodePtr node;

    void initNode();
    uavcan_linux::BlockingServiceClientPtr<ussp::payload::PayloadAdcsCommand> adcsset_client;

 public:
    AgentUAVCANClient(AgentConfig& config);

    void AdcsCommand(const org::openapitools::server::model::AdcsCommandRequest& req,
                     org::openapitools::server::model::AdcsCommandResponse& rsp);
};
