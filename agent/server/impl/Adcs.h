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
#include <ussp/payload/PayloadAdcsCommand.hpp>
#include <ussp/tfrs/ReceiverNavigationState.hpp>
#include "AdcsCommandResponse.h"
#include "AdcsCommandRequest.h"
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

inline namespace Adaptor {
    const std::string DecodeAcsMode(const ussp::payload::AcsMode& mode);
    const std::string Adapt(const ussp::payload::AcsMode& src);
    const org::openapitools::server::model::Adcs_quat_t Adapt(const ussp::payload::QuatT& src);
    const org::openapitools::server::model::Adcs_xyz_float_t Adapt(const ussp::payload::XyzFloatT& src);
    const org::openapitools::server::model::AdcsTarget Adapt(const ussp::payload::TargetT& src);
    const double Adapt(const double src);
    const float Adapt(const float src);
    const int Adapt(const int src);
    const unsigned int Adapt(const unsigned int src);
    const uint8_t EncodeAcsMode(const std::string& mode);
    const ussp::payload::QuatT Adapt(const org::openapitools::server::model::Adcs_quat_t& src); 
    const ussp::payload::XyzFloatT Adapt(const org::openapitools::server::model::Adcs_xyz_float_t& src);
    const ussp::payload::TargetT Adapt(const org::openapitools::server::model::AdcsTarget& src);

    const std::string NOOP = "NO-OP";
    const std::string BDOT = "BDOT";
    const std::string DETUMBLE = "DETUMBLE";
    const std::string SUNPOINT = "SUNPOINT";
    const std::string NADIRPOINTYAW = "NADIRPOINTYAW";
    const std::string SUNSPIN = "SUNSPIN";
    const std::string NADIRPOINTSUN = "NADIRPOINTSUN";
    const std::string SUNPOINTNADIR = "SUNPOINTNADIR";
    const std::string LATLONTRACK = "LATLONTRACK";
    const std::string INERTIALPOINT = "INERTIALPOINT";
    const std::string UNKNOWN = "UNKNOWN";
};
