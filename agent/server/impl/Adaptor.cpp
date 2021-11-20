/**
 * \file Adaptor.cpp
 *
 * \brief Routines to adapt UAVCAN structures to/from OAPI
 *
 * \copyright Copyright (c) 2021 Spire Global, Inc.
 */
#include "Adaptor.h"
#include "Log.h"

using namespace std;
using namespace org::openapitools::server::model;

// Adapt - overloaded function to convert between UAVCAN types
// and OAPI types.
inline namespace Adaptor {
const Adcs_quat_t
Adapt(const ussp::payload::QuatT& src) {
    Adcs_quat_t r;
    r.setQ1(src.q1);
    r.setQ2(src.q2);
    r.setQ3(src.q3);
    r.setQ4(src.q4);
    return r;
}

const org::openapitools::server::model::Adcs_xyz_float_t
Adapt(const ussp::payload::XyzFloatT& src) {
    org::openapitools::server::model::Adcs_xyz_float_t r;
    r.setX(src.x);
    r.setY(src.y);
    r.setZ(src.z);
    return r;
}

const org::openapitools::server::model::AdcsTarget
Adapt(const ussp::payload::TargetT& src) {
    org::openapitools::server::model::AdcsTarget t;
    t.setLat(src.lat);
    t.setLon(src.lon);
    return t;
}

const string
Adapt(const ussp::payload::AcsMode& src) {
    return DecodeAcsMode(src);
}

const double
Adapt(const double src) {
    return src;
}

const float
Adapt(const float src) {
    return src;
}

const int
Adapt(const int src) {
    return src;
}

const unsigned int
Adapt(const unsigned int src) {
    return src;
}

const ussp::payload::QuatT
Adapt(const org::openapitools::server::model::Adcs_quat_t& src) {
    ussp::payload::QuatT r;
    r.q1 = src.getQ1();
    r.q2 = src.getQ2();
    r.q3 = src.getQ3();
    r.q4 = src.getQ4();
    return r;
}

const ussp::payload::XyzFloatT
Adapt(const org::openapitools::server::model::Adcs_xyz_float_t& src) {
    ussp::payload::XyzFloatT r;
    r.x = src.getX();
    r.y = src.getY();
    r.z = src.getZ();
    return r;
}

const ussp::payload::TargetT
Adapt(const org::openapitools::server::model::AdcsTarget& src) {
    ussp::payload::TargetT t;
    t.lat = src.getLat();
    t.lon = src.getLon();
    return t;
}

const std::string DecodeAcsMode(const ussp::payload::AcsMode& src) {
    switch (src.mode) {
        case ussp::payload::AcsMode::NOOP:          return Adaptor::AcsMode::NOOP; break;
        case ussp::payload::AcsMode::BDOT:          return Adaptor::AcsMode::BDOT; break;
        case ussp::payload::AcsMode::DETUMBLE:      return Adaptor::AcsMode::DETUMBLE; break;
        case ussp::payload::AcsMode::SUNPOINT:      return Adaptor::AcsMode::SUNPOINT; break;
        case ussp::payload::AcsMode::NADIRPOINTYAW: return Adaptor::AcsMode::NADIRPOINTYAW; break;
        case ussp::payload::AcsMode::SUNSPIN:       return Adaptor::AcsMode::SUNSPIN; break;
        case ussp::payload::AcsMode::NADIRPOINTSUN: return Adaptor::AcsMode::NADIRPOINTSUN; break;
        case ussp::payload::AcsMode::SUNPOINTNADIR: return Adaptor::AcsMode::SUNPOINTNADIR; break;
        case ussp::payload::AcsMode::LATLONTRACK:   return Adaptor::AcsMode::LATLONTRACK; break;
        case ussp::payload::AcsMode::INERTIALPOINT: return Adaptor::AcsMode::INERTIALPOINT; break;
        default:                                    return Adaptor::AcsMode::UNKNOWN;
    }
}

const uint8_t EncodeAcsMode(const std::string& src) {
    if (src == Adaptor::AcsMode::NOOP) {
        return ussp::payload::AcsMode::NOOP;
    } else if (src == Adaptor::AcsMode::BDOT) {
        return ussp::payload::AcsMode::BDOT;
    } else if (src == Adaptor::AcsMode::DETUMBLE) {
        return ussp::payload::AcsMode::DETUMBLE;
    } else if (src == Adaptor::AcsMode::SUNPOINT) {
        return ussp::payload::AcsMode::SUNPOINT;
    } else if (src == Adaptor::AcsMode::NADIRPOINTYAW) {
        return ussp::payload::AcsMode::NADIRPOINTYAW;
    } else if (src == Adaptor::AcsMode::SUNSPIN) {
        return ussp::payload::AcsMode::SUNSPIN;
    } else if (src == Adaptor::AcsMode::NADIRPOINTSUN) {
        return ussp::payload::AcsMode::NADIRPOINTSUN;
    } else if (src == Adaptor::AcsMode::SUNPOINTNADIR) {
        return ussp::payload::AcsMode::SUNPOINTNADIR;
    } else if (src == Adaptor::AcsMode::LATLONTRACK) {
        return ussp::payload::AcsMode::LATLONTRACK;
    } else if (src == Adaptor::AcsMode::INERTIALPOINT) {
        return ussp::payload::AcsMode::INERTIALPOINT;
    } else {
        // anything else
        return 255;
    }
}

// OAPI request -> UAV request
const ussp::payload::PayloadAdcsCommand::Request
Adapt(const org::openapitools::server::model::AdcsCommandRequest& o_req) {
    ussp::payload::PayloadAdcsCommand::Request u_req;

    const string mode = o_req.getCommand();
    if (mode == AdcsCommand::IDLE) {
        u_req.adcs_command = u_req.ADCS_COMMAND_IDLE;
    } else if (mode == AdcsCommand::NADIR) {
        u_req.adcs_command = u_req.ADCS_COMMAND_NADIR;
    } else if (mode == AdcsCommand::TRACK) {
        u_req.adcs_command = u_req.ADCS_COMMAND_TRACK;
    } else {
        // should never happen here, should have been checked ahead of time.
        throw runtime_error("Cannot translate command " + mode);
    }

    if (o_req.apertureIsSet()) {
        const string ap = o_req.getAperture();
        if (ap.size() > u_req.aperture.capacity()) {
            throw runtime_error("Invalid aperture specification");
        }
        u_req.aperture = ap.c_str();
    } else {
        Log::debug("No aperture include in request");
    }

    if (o_req.targetIsSet()) {
        u_req.target.push_back(Adapt(o_req.getTarget()));
    }
    if (o_req.angleIsSet()) {
        u_req.angle.push_back(o_req.getAngle());
    }
    if (o_req.quatIsSet()) {
        u_req.quat.push_back(Adapt(o_req.getQuat()));
    }
    if (o_req.vectorIsSet()) {
        u_req.vector.push_back(Adapt(o_req.getVector()));
    }

    return u_req;
}

// UAV response -> OAPI response
const org::openapitools::server::model::AdcsCommandResponse
Adapt(const ussp::payload::PayloadAdcsCommand::Response& u_resp) {
    org::openapitools::server::model::AdcsCommandResponse o_resp;

    switch (u_resp.status) {
        case 0:
            o_resp.setStatus(Status::OK);
            break;
        case 1:
            o_resp.setStatus(Status::FAIL);
            break;
        default:
            o_resp.setStatus(Status::UNKNOWN);
            break;
    }

    o_resp.setReason(u_resp.reason.c_str());
    o_resp.setMode(Adaptor::DecodeAcsMode(u_resp.mode));
    if (u_resp.target.size() > 0) {
        o_resp.setTarget(Adaptor::Adapt(u_resp.target.front()));
    }
    if (u_resp.quat.size() > 0) {
        o_resp.setQuat(Adaptor::Adapt(u_resp.quat.front()));
    }
    if (u_resp.vector.size() > 0) {
        o_resp.setVector(Adaptor::Adapt(u_resp.vector.front()));
    }
    return o_resp;
}
};  // namespace Adaptor
