/**
 * \file Adaptor.h
 *
 * \brief Routines to adapt UAVCAN structures to/from OAPI
 *
 * \copyright Copyright (c) 2021 Spire Global, Inc.
 */
#pragma once
#include <string>
#include <ussp/payload/PayloadAdcsFeed.hpp>
#include <ussp/payload/PayloadAdcsCommand.hpp>
#include <ussp/tfrs/ReceiverNavigationState.hpp>
#include "AdcsCommandResponse.h"
#include "AdcsCommandRequest.h"
#include "AdcsResponse.h"
#include "TfrsResponse.h"

inline namespace Adaptor {
    const std::string DecodeAcsMode(const ussp::payload::AcsMode& mode);
    const std::string Adapt(const ussp::payload::AcsMode& src);
    const org::openapitools::server::model::Adcs_quat_t Adapt(const ussp::payload::QuatT& src);
    const org::openapitools::server::model::Adcs_xyz_float_t
          Adapt(const ussp::payload::XyzFloatT& src);
    const org::openapitools::server::model::AdcsTarget Adapt(const ussp::payload::TargetT& src);
    const double Adapt(const double src);
    const float Adapt(const float src);
    const int Adapt(const int src);
    const unsigned int Adapt(const unsigned int src);
    const uint8_t EncodeAcsMode(const std::string& mode);
    const ussp::payload::QuatT Adapt(const org::openapitools::server::model::Adcs_quat_t& src);
    const ussp::payload::XyzFloatT
          Adapt(const org::openapitools::server::model::Adcs_xyz_float_t& src);
    const ussp::payload::TargetT Adapt(const org::openapitools::server::model::AdcsTarget& src);

    // OAPI request -> UAV request
    const ussp::payload::PayloadAdcsCommand::Request
          Adapt(const org::openapitools::server::model::AdcsCommandRequest& src);
    // UAV response -> OAPI response
    const org::openapitools::server::model::AdcsCommandResponse
          Adapt(const ussp::payload::PayloadAdcsCommand::Response& src);

    // AcsMode string constants
    namespace AcsMode {
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
    };  // namespace AcsMode

    namespace Status {
    const std::string FAIL = "FAIL";
    const std::string OK = "OK";
    const std::string UNKNOWN = "UNKNOWN";
    };  // namespace Status

    namespace AdcsCommand {
    const std::string IDLE = "IDLE";
    const std::string NADIR = "NADIR";
    const std::string TRACK = "TRACK";
    };  // namespace AdcsCommand
};
