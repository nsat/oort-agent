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
    using ussp::payload::AcsMode ;
    switch (src.mode) {
        case AcsMode::NOOP:          return Adaptor::NOOP; break;
        case AcsMode::BDOT:          return Adaptor::BDOT; break;
        case AcsMode::DETUMBLE:      return Adaptor::DETUMBLE; break;
        case AcsMode::SUNPOINT:      return Adaptor::SUNPOINT; break;
        case AcsMode::NADIRPOINTYAW: return Adaptor::NADIRPOINTYAW; break;
        case AcsMode::SUNSPIN:       return Adaptor::SUNSPIN; break;
        case AcsMode::NADIRPOINTSUN: return Adaptor::NADIRPOINTSUN; break;
        case AcsMode::SUNPOINTNADIR: return Adaptor::SUNPOINTNADIR; break;
        case AcsMode::LATLONTRACK:   return Adaptor::LATLONTRACK; break;
        case AcsMode::INERTIALPOINT: return Adaptor::INERTIALPOINT; break;
        default:                     return Adaptor::UNKNOWN;
    }
}

const uint8_t EncodeAcsMode(const std::string& src) {
    using ussp::payload::AcsMode ;
    if (src == Adaptor::NOOP) { 
        return AcsMode::NOOP;
    } else if (src == Adaptor::BDOT) { 
        return AcsMode::BDOT;
    } else if (src == Adaptor::DETUMBLE) {
        return AcsMode::DETUMBLE;
    } else if (src == Adaptor::SUNPOINT) {
        return AcsMode::SUNPOINT;
    } else if (src == Adaptor::NADIRPOINTYAW) {
        return AcsMode::NADIRPOINTYAW;
    } else if (src == Adaptor::SUNSPIN) {
        return AcsMode::SUNSPIN;
    } else if (src == Adaptor::NADIRPOINTSUN) {
        return AcsMode::NADIRPOINTSUN;
    } else if (src == Adaptor::SUNPOINTNADIR) {
        return AcsMode::SUNPOINTNADIR;
    } else if (src == Adaptor::LATLONTRACK) {
        return AcsMode::LATLONTRACK;
    } else if (src == Adaptor::INERTIALPOINT) {
        return AcsMode::INERTIALPOINT;
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
    if (mode == "IDLE") {
        u_req.adcs_command = u_req.ADCS_COMMAND_IDLE;
    } else if (mode == "NADIR") {
        u_req.adcs_command = u_req.ADCS_COMMAND_NADIR;
    } else if (mode == "TRACK") {
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
            o_resp.setStatus("OK");
            break;
        case 1:
            o_resp.setStatus("FAIL");
            break;
        default:
            o_resp.setStatus("UNKNOWN");
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

}; // namespace Adaptor
