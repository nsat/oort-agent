/**
 * AgentUAVCANServer.h
 *
 * Agent UAVCAN server task.  
 *
 * Copyright (c) 2021 Spire Global, Inc.
 */
#pragma once

#include <semaphore.h>
#include <atomic>
#include <string>
#include <thread>  // NOLINT(build/c++11)

#include "Adcs.h"

#include <uavcan_linux/uavcan_linux.hpp>
#include <uavcan/helpers/ostream.hpp>
#include <ussp/payload/PayloadHealthCheck.hpp>

/**
 * \brief Agent UAVCAN server
 * 
 * The UAVCAN server listens on the specified CAN interface
 * and exposes a UAVCAN interface using the DSDL defined in agent/server/ussp.
 * 
 */
class AgentUAVCANServer {
    std::thread server_thread;
    std::atomic<bool> thread_running;
    std::string can_interface;
    unsigned int uavcan_node_id;
    std::string healthcheck_cmd;

    uavcan_linux::NodePtr initNode();
    void serverTask();
    void parseHealthcheckResponse(std::string hk_output,
                                  ussp::payload::PayloadHealthCheck::Response& rsp);

 public:
    AdcsManager m_mgr;
    AgentUAVCANServer(std::string iface, unsigned int node_id);

    void start();
    void stop();

    // Public for testing purposes
    void healthCheckHandler(
        const uavcan::ReceivedDataStructure<ussp::payload::PayloadHealthCheck::Request>& req,
        ussp::payload::PayloadHealthCheck::Response& rsp);

    // Only used for testing
    void setPayloadHealthcheckCommand(std::string cmd);
};
