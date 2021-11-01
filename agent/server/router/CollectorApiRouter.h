/**
 * main-api-server.cpp
 *
 * OORT Agent main()
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>
#include "onion/url.hpp"
#include "onion/handler.hpp"

#include "InfoRequest.h"

using org::openapitools::server::model::InfoRequest;

class CollectorApiRouter : public Onion::Url {
 public:
    CollectorApiRouter();
    virtual void ping(Onion::Response &response) = 0;
    virtual void info(InfoRequest &request, Onion::Response &response) = 0;
    virtual void meta(
        const std::string &uuid, Onion::Response &response) = 0;
};
