/**
 * AgentUAVCANServer.cpp
 *
 * Agent UAVCAN server task
 *
 * Copyright (c) 2021 Spire Global, Inc.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <system_error>  // NOLINT(build/c++11)
#include <utility>
#include <vector>

#include "Log.h"
#include "AgentUAVCANServer.h"
#include "Utils.h"
#include <ussp/payload/PayloadAdcsFeed.hpp>
#include <ussp/tfrs/ReceiverNavigationState.hpp>

#include "nlohmann/json.hpp"

using namespace std;
using nlohmann::json;

#define MAX_HEALTHCHECK_OUTPUT_SIZE 65536

#define HEALTHCHECK_CMD "/usr/bin/payload_healthcheck"
#define HK_COMMAND_TIMEOUT 1
#define INVALID_OUTPUT_RESPONSE_CODE 22
#define RUNTIME_ERROR_RESPONSE_CODE 50
#define SYSTEM_ERROR_RESPONSE_CODE 51
#define UNKNOWN_ERROR_RESPONSE_CODE 52

#define HK_INTERFACE_VERSION 1
#define HK_MAX_ARRAY_LEN 255

#define VALIDATE_ARRAY(obj, name) do { \
        if (!obj.at(name).is_array()) { \
            throw runtime_error("Invalid type for " name); \
        } \
        if (obj.at(name).size() > HK_MAX_ARRAY_LEN) { \
            throw runtime_error("Invalid array size for " name); \
        } \
    } while (0)

void AgentUAVCANServer::parseHealthcheckResponse(std::string hk_output,
        ussp::payload::PayloadHealthCheck::Response& rsp) {
    json j = json::parse(hk_output);

    int64_t iface_version;
    j.at("version").get_to(iface_version);
    if (iface_version != HK_INTERFACE_VERSION) {
        throw runtime_error("Invalid version in JSON healthcheck response");
    }
    VALIDATE_ARRAY(j, "int_metrics");
    VALIDATE_ARRAY(j, "float_metrics");
    VALIDATE_ARRAY(j, "int_metric_counts");
    VALIDATE_ARRAY(j, "float_metric_counts");

    if (!j.at("schema_version").is_number_unsigned()) {
        throw runtime_error("Invalid type for schema_version");
    }
    if (!j.at("payload_timestamp").is_number_unsigned()) {
        throw runtime_error("Invalid type for payload_timestamp");
    }
    if (!j.at("need_hard_reset").is_boolean()) {
        throw runtime_error("Invalid type for need_hard_reset");
    }

    j.at("schema_version").get_to(rsp.schema_version);
    j.at("payload_timestamp").get_to(rsp.timestamp);
    j.at("need_hard_reset").get_to(rsp.need_hard_reset);

    for (auto& it : j.at("int_metrics")) {
        rsp.imetric.push_back(it);
    }
    for (auto& it : j.at("int_metric_counts")) {
        rsp.imetric_feeds.push_back(it);
    }
    for (auto& it : j.at("float_metrics")) {
        rsp.fmetric.push_back(it);
    }
    for (auto& it : j.at("float_metric_counts")) {
        rsp.fmetric_feeds.push_back(it);
    }
}

void AgentUAVCANServer::healthCheckHandler(
        const uavcan::ReceivedDataStructure<ussp::payload::PayloadHealthCheck::Request>& req,
        ussp::payload::PayloadHealthCheck::Response& rsp) {
    (void)req;

    std::string hk_output;
    const char *const argv[2] = { tailname(healthcheck_cmd).c_str(), NULL};
    try {
        rsp.health_state = (uint8_t) runProcessGetOutput(healthcheck_cmd, argv,
                                                        MAX_HEALTHCHECK_OUTPUT_SIZE,
                                                        HK_COMMAND_TIMEOUT, hk_output);
        parseHealthcheckResponse(hk_output, rsp);
    }
    catch (json::parse_error& e) {
        rsp.response_code = INVALID_OUTPUT_RESPONSE_CODE;
        Log::error("HealthCheck parse_error ?: ?", e.id, e.what());
    }
    catch (json::type_error& e) {
        rsp.response_code = INVALID_OUTPUT_RESPONSE_CODE;
        Log::error("HealthCheck type_error ?: ?", e.id, e.what());
    }
    catch (system_error& e) {
        rsp.response_code = SYSTEM_ERROR_RESPONSE_CODE;
        Log::error("HealthCheck system_error: ?", e.what());
    }
    catch (runtime_error& e) {
        rsp.response_code = RUNTIME_ERROR_RESPONSE_CODE;
        Log::error("HealthCheck runtime_error: ?", e.what());
    }
    catch (std::exception& e) {
        rsp.response_code = UNKNOWN_ERROR_RESPONSE_CODE;
        Log::error("HealthCheck exception: ?", e.what());
    }
}

uavcan_linux::NodePtr AgentUAVCANServer::initNode() {
    std::vector<std::string> ifaces(1);
    ifaces[0] = can_interface;
    auto node = uavcan_linux::makeNode(ifaces, "com.spire.linux_agent",
                                       uavcan::protocol::SoftwareVersion(),
                                       uavcan::protocol::HardwareVersion(),
                                       (uavcan::NodeID) uavcan_node_id);
    node->getScheduler().setDeadlineResolution(uavcan::MonotonicDuration::fromMSec(100));
    node->setModeOperational();

    return node;
}

AgentUAVCANServer::AgentUAVCANServer(std::string iface, unsigned int node_id) {
    thread_running = false;
    can_interface = iface;
    uavcan_node_id = node_id;
    setPayloadHealthcheckCommand(HEALTHCHECK_CMD);
}

void AgentUAVCANServer::serverTask() {
    Log::setThreadName("uavcan_server");
    Log::info("UAVCAN server starting on ? with node ID ?", can_interface, uavcan_node_id);
    uavcan_linux::NodePtr node = initNode();
    // HealthCheck
    auto health_srv = node->makeServiceServer<ussp::payload::PayloadHealthCheck>(
        [&](const uavcan::ReceivedDataStructure<ussp::payload::PayloadHealthCheck::Request>& req,
            ussp::payload::PayloadHealthCheck::Response& rsp) {
                Log::debug("HEALTHCHECK");
                healthCheckHandler(req, rsp);
            });
    auto adcs_sub = node->makeSubscriber<ussp::payload::PayloadAdcsFeed>(
        [this](const uavcan::ReceivedDataStructure<ussp::payload::PayloadAdcsFeed>& msg) {
            Log::info("Received ADCS broadcast for time ?", msg.unix_timestamp);
            m_mgr.setAdcs(msg);
        });
    auto tfrs_sub = node->makeSubscriber<ussp::tfrs::ReceiverNavigationState>(
        [this](const uavcan::ReceivedDataStructure<ussp::tfrs::ReceiverNavigationState>& msg) {
            Log::debug("Received TFRS broadcast for time ?", msg.utc_time);
            m_mgr.setTfrs(msg);
        });

    while (thread_running) {
        node->spin(uavcan::MonotonicDuration::fromMSec(1000));
    }
    Log::info("UAVCAN server task exiting");
}

void AgentUAVCANServer::start() {
    thread_running = true;
    thread worker(&AgentUAVCANServer::serverTask, this);
    ostringstream s("");
    s << worker.get_id();
    Log::info("UAVCAN server worker thread id = ?", s.str());
    server_thread = std::move(worker);
}

void AgentUAVCANServer::stop() {
    thread_running = false;
    server_thread.join();
}

void AgentUAVCANServer::setPayloadHealthcheckCommand(std::string cmd) {
    healthcheck_cmd = cmd;
}
