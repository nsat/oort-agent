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

#include "Adcs.h"
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
    adcsset_client = node->makeBlockingServiceClient<ussp::payload::PayloadAdcsCommand>();
}

void AgentUAVCANClient::AdcsCommand(const AdcsCommandRequest& req, AdcsCommandResponse& rsp) {
    ussp::payload::PayloadAdcsCommand::Request can_req;
    int can_stat;

    auto timeout = uavcan::MonotonicDuration::fromMSec(UAVCLIENT_TIMEOUT);

    org::openapitools::server::model::AdcsCommandResponse uresp;
    auto const mode = req.getCommand();
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
    if (req.apertureIsSet()) {
        auto ap = req.getAperture();
        if (ap.size() > can_req.aperture.capacity()) {
            rsp.setStatus("FAIL");
            rsp.setReason("Invalid aperture specification");
            return;
        }
        can_req.aperture = ap.c_str();
    } else {
        Log::debug("No aperture");
    }

    if (req.targetIsSet()) {
        auto t = req.getTarget();
        ussp::payload::TargetT tgt;
        tgt.lat = t.getLat();
        tgt.lon = t.getLon();
        Log::debug("latlon ?, ?", t.getLat(), t.getLon());
        can_req.target.push_back(tgt);
    }
    if (req.angleIsSet()) {
        can_req.angle.push_back(req.getAngle());
    }
    if (req.quatIsSet()) {
        auto rq = req.getQuat();
        ussp::payload::QuatT q;
        q.q1 = rq.getQ1();
        q.q2 = rq.getQ2();
        q.q3 = rq.getQ3();
        q.q4 = rq.getQ4();
        can_req.quat.push_back(q);
    }
    if (req.vectorIsSet()) {
        auto rv = req.getVector();
        ussp::payload::XyzFloatT v;
        v.x = rv.getX();
        v.y = rv.getY();
        v.z = rv.getZ();
        can_req.vector.push_back(v);
    }

    can_stat = adcsset_client->blockingCall(config.getShimNodeID(), can_req, timeout);
    ussp::payload::PayloadAdcsCommand::Response can_rsp = adcsset_client->getResponse();

    if (can_stat <= 0 || !adcsset_client->wasSuccessful()) {  
        // call failed
        rsp.setStatus("FAIL");
        rsp.setReason("Error contacting service: " + to_string(can_stat));
        return;
    }

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
    rsp.setMode(Adaptor::DecodeAcsMode(can_rsp.mode));
    if (can_rsp.target.size() > 0) {
        rsp.setTarget(Adaptor::Adapt(can_rsp.target.front()));
    }
    if (can_rsp.quat.size() > 0) {
        rsp.setQuat(Adaptor::Adapt(can_rsp.quat.front()));
    }
    if (can_rsp.vector.size() > 0) {
        rsp.setVector(Adaptor::Adapt(can_rsp.vector.front()));
    }
}
