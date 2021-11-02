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

#include "Log.h"
#include "AgentUAVCANClient.h"
#include "Utils.h"

// #include "nlohmann/json.hpp"

using namespace std;
// using nlohmann::json;

#define INVALID_OUTPUT_RESPONSE_CODE 22
#define RUNTIME_ERROR_RESPONSE_CODE 50
#define SYSTEM_ERROR_RESPONSE_CODE 51
#define UNKNOWN_ERROR_RESPONSE_CODE 52

#define HK_INTERFACE_VERSION 1
#define HK_MAX_ARRAY_LEN 255

void AgentUAVCANClient::initNode() {
    std::vector<std::string> ifaces(1);
    ifaces[0] = can_interface;
    node = uavcan_linux::makeNode(ifaces, "com.spire.linux_agent",
                                  uavcan::protocol::SoftwareVersion(),
                                  uavcan::protocol::HardwareVersion(),
                                  static_cast<uavcan::NodeID>(uavcan_node_id));
    node->getScheduler().setDeadlineResolution(uavcan::MonotonicDuration::fromMSec(100));
    node->setModeOperational();
}


AgentUAVCANClient::AgentUAVCANClient(std::string iface, unsigned int node_id) {
    can_interface = iface;
    uavcan_node_id = node_id;
    initNode();
    adcsset_client = node->makeBlockingServiceClient<ussp::payload::PayloadAdcsCommand>();
}

void AgentUAVCANClient::AdcsSet(const AdcsSetRequest& req, AdcsSetResponse& rsp) {
    ussp::payload::PayloadAdcsCommand::Request can_req;
    int can_stat;

    auto timeout = uavcan::MonotonicDuration::fromMSec(20);

    org::openapitools::server::model::AdcsSetResponse uresp;
    auto const mode = req.getCommand().getCommand();
    if (mode == "IDLE") {
        can_req.adcs_command = can_req.ADCS_COMMAND_IDLE;
    } else if (mode == "NADIR") {
        can_req.adcs_command = can_req.ADCS_COMMAND_NADIR;
    } else if (mode == "TRACK") {
        can_req.adcs_command = can_req.ADCS_COMMAND_TRACK;
    } else  {
        rsp.setStatus("FAIL");
        rsp.setReason("Unknown ADCS comand mode " + mode);
        return;
    }

    // fill(can_req.aperture.begin(), can_req.aperture.end(), 0);
    copy_n(req.getAperture().begin(),
           min(static_cast<unsigned int>(req.getAperture().size()),
               static_cast<unsigned int>(can_req.aperture.capacity())),
           can_req.aperture.begin());

    Log::debug("latlon ");
    auto t = req.getTarget();
    can_req.target.lat = t.getLat();
    can_req.target.lon = t.getLon();

    can_stat = adcsset_client->blockingCall(SBRAIN_NODE_ID, can_req, timeout);
    ussp::payload::PayloadAdcsCommand::Response can_rsp = adcsset_client->getResponse();

    switch (can_rsp.status) {
        case 0:
            rsp.setStatus("OK");
            break;
        case 1:
            rsp.setStatus("FAIL");
            break;
        default:
            rsp.setStatus("UNKNOWN");
            break;
    }

    rsp.setReason(can_rsp.reason.c_str());
}
