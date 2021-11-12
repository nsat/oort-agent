/**
 * Adcs.h
 * 
 * Adcs and Tfrs conversion and caching utility.
 * 
 * Copyright (c) 2021 Spire Global, Inc.
 */
#pragma once

#include <chrono>  // NOLINT(build/c++11)

#include <ussp/payload/PayloadAdcsFeed.hpp>
// #include <ussp/payload/PayloadAdcsCommand.hpp>
#include <ussp/tfrs/ReceiverNavigationState.hpp>
// #include "AdcsCommandResponse.h"
// #include "AdcsCommandRequest.h"
#include "AdcsResponse.h"
#include "TfrsResponse.h"

class AdcsManager {
    org::openapitools::server::model::AdcsResponse m_adcs;
    std::chrono::time_point<std::chrono::system_clock> m_adcs_mtime;
    org::openapitools::server::model::TfrsResponse m_tfrs;
    std::chrono::time_point<std::chrono::system_clock> m_tfrs_mtime;
 public:
    void setAdcs(const ussp::payload::PayloadAdcsFeed& adcs);
    void setTfrs(const ussp::tfrs::ReceiverNavigationState& tfrs);
    org::openapitools::server::model::AdcsResponse getAdcs();
    org::openapitools::server::model::TfrsResponse getTfrs();
};
