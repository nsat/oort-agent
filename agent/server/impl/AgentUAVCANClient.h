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
#include <ussp/payload/PayloadAdcsHk.hpp>
#include <ussp/payload/PayloadAdcsSet.hpp>
#include "AdcsResponse.h"
#include "AdcsSetRequest.h"
#include "AdcsSetResponse.h"

const int SBRAIN_NODE_ID = 11;

/**
 * \brief Agent UAVCAN client
 * 
 * The UAVCAN client makes calls over UAVCAN to other services (generally
 * SBrain) and allows those services to be called from Agent handlers.
 * 
 */
class AgentUAVCANClient {
    std::string can_interface;
    unsigned int uavcan_node_id;
    uavcan_linux::NodePtr node;

    void initNode();
    uavcan_linux::BlockingServiceClientPtr<ussp::payload::PayloadAdcsHk> adcshk_client;
    uavcan_linux::BlockingServiceClientPtr<ussp::payload::PayloadAdcsSet> adcsset_client;

 public:
    AgentUAVCANClient(std::string iface, unsigned int node_id);

    void AdcsHk(org::openapitools::server::model::AdcsResponse& rsp);
    void AdcsSet(const org::openapitools::server::model::AdcsSetRequest& req,
                 org::openapitools::server::model::AdcsSetResponse& rsp);
};
