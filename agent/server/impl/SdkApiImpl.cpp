/**
 * SdkApiImpl.cpp
 *
 * OORT Agent SDK Api interface.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#include "SdkApiImpl.h"
#include "Utils.h"

using namespace org::openapitools::server::model;
using namespace Onion;

SdkApiImpl::SdkApiImpl(Agent *agent) {
        m_agent = agent;
     }

void SdkApiImpl::query_available_files(
    const std::string &topic, Response &response
) {
    auto resp = m_agent->query_available(topic);
    deliverResponse(response, resp);
}

void SdkApiImpl::retrieve_file(
    const RetrieveFileRequest &retrieveFileRequest, Response &response
) {
    auto resp = m_agent->retrieve_file(retrieveFileRequest);
    deliverResponse(response, resp);
}

void SdkApiImpl::send_file(
    const SendFileRequest &sendFileRequest, Response &response
) {
    auto resp = m_agent->send_file(sendFileRequest);
    deliverResponse(response, resp);
}

void SdkApiImpl::adcs_get(
    Onion::Response &response
) {
    auto resp = m_agent->adcs_get();
    deliverResponse(response, resp);
}

void SdkApiImpl::adcs_post(
    const AdcsCommandRequest &adcsCommandRequest, Onion::Response &response
) {
    auto resp = m_agent->adcs_command(adcsCommandRequest);
    deliverResponse(response, resp);
}

void SdkApiImpl::tfrs_get(
    Onion::Response &response
) {
    auto resp = m_agent->tfrs_get();
    deliverResponse(response, resp);
}
