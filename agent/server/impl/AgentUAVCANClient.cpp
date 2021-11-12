/**
 * AgentUAVCANClient.cpp
 *
 * Agent UAVCAN client operations
 *
 * Copyright (c) 2021 Spire Global, Inc.
 */

#include <unistd.h>
#include <sys/types.h>
#include <algorithm>
#include <sstream>
#include <system_error>  // NOLINT(build/c++11)
#include <vector>

#include "Adaptor.h"
#include "Adcs.h"
#include "Log.h"
#include "AgentUAVCANClient.h"
#include "Utils.h"

using namespace std;

// can we use use same node as server?
void AgentUAVCANClient::initNode() {
    std::vector<std::string> ifaces(1);
    ifaces[0] = config.getCANInterface();
    unsigned int uavcan_node_id = config.getUAVCANNodeID();
    node = uavcan_linux::makeNode(ifaces, "com.spire.linux_agent",
                                  uavcan::protocol::SoftwareVersion(),
                                  uavcan::protocol::HardwareVersion(),
                                  static_cast<uavcan::NodeID>(uavcan_node_id));
    node->getScheduler().setDeadlineResolution(uavcan::MonotonicDuration::fromMSec(100));
    node->setModeOperational();
}

AgentUAVCANClient::AgentUAVCANClient(AgentConfig& config) : config(config) {
    initNode();
    adcscommand_client = node->makeBlockingServiceClient<ussp::payload::PayloadAdcsCommand>();
}

void AgentUAVCANClient::AdcsCommand(const AdcsCommandRequest& req, AdcsCommandResponse& rsp) {
    ussp::payload::PayloadAdcsCommand::Request can_req;
    int can_stat;

    auto timeout = uavcan::MonotonicDuration::fromMSec(UAVCLIENT_TIMEOUT);

    try {
        can_req = Adapt(req);
    } catch (const runtime_error e) {
        // the conversion failed
        Log::error("Failed creating UAVCAN request from OAPI request: ?", e.what());
        rsp.setStatus("FAIL");
        rsp.setReason(e.what());
        return;
    }

    can_stat = adcscommand_client->blockingCall(config.getShimNodeID(), can_req, timeout);
    if (can_stat <= 0 || !adcscommand_client->wasSuccessful()) {  
        // call failed
        Log::error("Failed contacting UAVCAN PayloadAdcsCommand service: ?", can_stat);
        rsp.setStatus("FAIL");
        rsp.setReason("Error contacting service: " + to_string(can_stat));
        return;
    }
    ussp::payload::PayloadAdcsCommand::Response can_rsp = adcscommand_client->getResponse();
    
    rsp = Adapt(can_rsp);
}
