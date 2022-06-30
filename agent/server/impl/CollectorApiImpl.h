/**
 * CollectorApiImpl.h
 * 
 * OORT Agent Collector Interface
 * 
 * Copyright (c) 2020 Spire Global, Inc.
*/
#pragma once

#include <string>
#include "onion/url.hpp"
#include "onion/handler.hpp"
#include "onion/request.hpp"
#include "onion/response.hpp"

#include "CollectorApiRouter.h"
#include "InfoRequest.h"
#include "Agent.h"

using org::openapitools::server::model::InfoRequest;

class CollectorApiImpl : public CollectorApiRouter {
 public:
    explicit CollectorApiImpl(Agent *agent);
    ~CollectorApiImpl() {}

    void ping(Onion::Response &response);
    void info(InfoRequest &request, Onion::Response &response);
    void meta(const std::string &uuid, Onion::Response &response);

 private:
    Agent *m_agent;
};
