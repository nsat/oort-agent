/**
 * CollectorApiImpl.cpp
 * 
 * OORT Agent Collector Interface
 * 
 * Copyright (c) 2020 Spire Global, Inc.
*/

#include "CollectorApiImpl.h"
#include <string>

#include "Agent.h"
#include "Utils.h"
#include "InfoRequest.h"

using namespace org::openapitools::server::model;
using namespace Onion;

CollectorApiImpl::CollectorApiImpl(Agent *agent)
    : CollectorApiRouter() {
    m_agent = agent;
}

void CollectorApiImpl::info(InfoRequest &request, Response &response) {
    auto resp = m_agent->collector_info(request);
    deliverResponse(response, resp);
}

void CollectorApiImpl::meta(const std::string &uuid, Onion::Response &response) {
    auto resp = m_agent->meta(uuid);
    deliverResponse(response, resp);
}
