/**
 * AgentUAVCANClient.cpp
 *
 * Agent UAVCAN client operations
 *
 * Copyright (c) 2021 Spire Global, Inc.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <system_error>
#include <vector>

#include "Log.h"
#include "AgentUAVCANClient.h"
#include "Utils.h"

// #include "nlohmann/json.hpp"

using namespace std;
// using nlohmann::json;

#define INVALID_OUTPUT_RESPONSE_CODE 22
#define RUNTIME_ERROR_RESPONSE_CODE 50
#define SYSTEM_ERROR_RESPONSE_CODE 51
#define UNKNOWN_ERROR_RESPONSE_CODE 52

#define HK_INTERFACE_VERSION 1
#define HK_MAX_ARRAY_LEN 255


// Adapt - overloaded function to convert between UAVCAN types
// and OAPI types.
static org::openapitools::server::model::Adcs_quat_t
Adapt(ussp::payload::QuatT_<0> src) {
    org::openapitools::server::model::Adcs_quat_t r;
    r.setQ1(src.q1);
    r.setQ2(src.q2);
    r.setQ3(src.q3);
    r.setQ4(src.q4);
    return r;
};

static org::openapitools::server::model::Adcs_xyz_float_t
Adapt(ussp::payload::XyzFloatT_<0> src) {
    org::openapitools::server::model::Adcs_xyz_float_t r;
    r.setX(src.x);
    r.setY(src.y);
    r.setZ(src.z);
    return r;
};

static org::openapitools::server::model::Adcs_xyz_int16_t
Adapt(ussp::payload::XyzInt16T_<0>& src) {
    org::openapitools::server::model::Adcs_xyz_int16_t r;
    r.setX(src.x);
    r.setY(src.y);
    r.setZ(src.z);
    return r;
};

static ussp::payload::QuatT_<0> 
Adapt(const org::openapitools::server::model::Adcs_quat_t& src) {
    ussp::payload::QuatT_<0> r;
    r.q1 = src.getQ1();
    r.q2 = src.getQ2();
    r.q3 = src.getQ3();
    r.q4 = src.getQ4();
    return r;
};

static ussp::payload::XyzFloatT_<0>
Adapt(const org::openapitools::server::model::Adcs_xyz_float_t& src) {
    ussp::payload::XyzFloatT_<0> r;
    r.x = src.getX();
    r.y = src.getY();
    r.z = src.getZ();
    return r;
};

static ussp::payload::XyzInt16T_<0>
Adapt(const org::openapitools::server::model::Adcs_xyz_int16_t& src) {
    ussp::payload::XyzInt16T_<0> r;
    r.x = src.getX();
    r.y = src.getY();
    r.z = src.getZ();
    return r;
};


void AgentUAVCANClient::initNode() {
    std::vector<std::string> ifaces(1);
    ifaces[0] = can_interface;
    node = uavcan_linux::makeNode(ifaces, "com.spire.linux_agent",
                                  uavcan::protocol::SoftwareVersion(),
                                  uavcan::protocol::HardwareVersion(),
                                  static_cast<uavcan::NodeID>(uavcan_node_id));
    node->getScheduler().setDeadlineResolution(uavcan::MonotonicDuration::fromMSec(100));
    node->setModeOperational();
}


AgentUAVCANClient::AgentUAVCANClient(std::string iface, unsigned int node_id) {
    can_interface = iface;
    uavcan_node_id = node_id;
    initNode();
    adcshk_client = node->makeBlockingServiceClient<ussp::payload::PayloadAdcsHk>();
    adcsset_client = node->makeBlockingServiceClient<ussp::payload::PayloadAdcsSet>();
}

void AgentUAVCANClient::AdcsHk(AdcsResponse& rsp) {
    ussp::payload::PayloadAdcsHk::Request can_req;
    int can_stat;

    auto timeout = uavcan::MonotonicDuration::fromMSec(20);

    org::openapitools::server::model::AdcsHk hk;
    can_stat = adcshk_client->blockingCall(SBRAIN_NODE_ID, can_req, timeout);
    ussp::payload::PayloadAdcsHk::Response can_rsp = adcshk_client->getResponse();

    // copy all the fields!!
    hk.setUnixTimestamp(can_rsp.unix_timestamp);
    hk.setControlErrorQ(       Adapt(can_rsp.control_error_q));
    hk.setQBiEst(              Adapt(can_rsp.q_bi_est));
    hk.setQBoEst(              Adapt(can_rsp.q_bo_est));
    hk.setQcf(                 Adapt(can_rsp.qcf));
    hk.setControlBdot(         Adapt(can_rsp.control_bdot));
    hk.setControlErrorOmega(   Adapt(can_rsp.control_error_omega));
    hk.setGyroBiasEkfGyrovec(  Adapt(can_rsp.gyro_bias_ekf_gyrovec));
    hk.setImu1Gyro(            Adapt(can_rsp.imu1_gyro));
    hk.setImu2Gyro(            Adapt(can_rsp.imu2_gyro));
    hk.setLatlontrackBodyVector(Adapt(can_rsp.latlontrack_body_vector));
    hk.setMagEci(              Adapt(can_rsp.mag_eci));
    hk.setMagVector(           Adapt(can_rsp.mag_vector));
    hk.setMtqDipoleCmd(        Adapt(can_rsp.mtq_dipole_cmd));
    hk.setNadirVector(         Adapt(can_rsp.nadir_vector));
    hk.setOmegaBiEst(          Adapt(can_rsp.omega_bi_est));
    hk.setOmegaBoEst(          Adapt(can_rsp.omega_bo_est));
    hk.setScREci(              Adapt(can_rsp.sc_r_eci));
    hk.setScVEci(              Adapt(can_rsp.sc_v_eci));
    hk.setSunEci(              Adapt(can_rsp.sun_eci));
    hk.setSunVector(           Adapt(can_rsp.sun_vector));
    hk.setSunpointBodyVector(  Adapt(can_rsp.sunpoint_body_vector));
    hk.setWheelTorqueCmdAcs(   Adapt(can_rsp.wheel_torque_cmd_acs));
    hk.setWheelTorqueCmdDreMapped(Adapt(can_rsp.wheel_torque_cmd_dre));
    hk.setMtqDutyCycle(        Adapt(can_rsp.mtq_duty_cycle));
    hk.setWheelSpeedCmdMapped( Adapt(can_rsp.wheel_speed_cmd));
    hk.setWheelSpeed(          Adapt(can_rsp.wheel_speed));
    hk.setLatlontrackLat(can_rsp.latlontrack_lat);
    hk.setLatlontrackLon(can_rsp.latlontrack_lon);
    hk.setYawsun(can_rsp.yawsun);
    hk.setCssUsedBitmask(can_rsp.css_used_bitmask);
    hk.setLeaseTimeRemaining(can_rsp.lease_time_remaining);
    hk.setStatusBitmask(can_rsp.status_bitmask);
    hk.setAcsModeActive(can_rsp.acs_mode_active);
    hk.setAcsModeCmd(can_rsp.acs_mode_cmd);
    hk.setEkfGyroType(can_rsp.ekf_gyro_type);
    hk.setOpmode(can_rsp.opmode);
    hk.setAttitudeEstType(can_rsp.attitude_est_type);


    AdcsMode mode;
    mode.setMode("IDLE");

    rsp.setMode(mode);
    rsp.setController("you");
    rsp.setAge(120);
    rsp.setHk(hk);
}

void AgentUAVCANClient::AdcsSet(const AdcsSetRequest& req, AdcsSetResponse& rsp) {
    ussp::payload::PayloadAdcsSet::Request can_req;
    int can_stat;

    auto timeout = uavcan::MonotonicDuration::fromMSec(20);

    org::openapitools::server::model::AdcsSetResponse uresp;
    // validate?
    // fill(can_req.adcs_mode.begin(), can_req.adcs_mode.end(), 0);
    copy_n(req.getMode().getMode().begin(), 
           min(static_cast<unsigned int>(req.getMode().getMode().size()), static_cast<unsigned int>(can_req.adcs_mode.capacity())),
           can_req.adcs_mode.begin());

    // fill(can_req.antenna.begin(), can_req.antenna.end(), 0);
    copy_n(req.getAntenna().begin(),
           min(static_cast<unsigned int>(req.getAntenna().size()), static_cast<unsigned int>(can_req.antenna.capacity())),
           can_req.antenna.begin());

    Log::debug("latlon ");
    auto t = req.getTarget();
    can_req.target.lat = t.getLat();
    can_req.target.lon = t.getLon();

    can_stat = adcsset_client->blockingCall(SBRAIN_NODE_ID, can_req, timeout);
    ussp::payload::PayloadAdcsSet::Response can_rsp = adcsset_client->getResponse();
    
    switch (can_rsp.status) {
        case 0: 
            rsp.setStatus("OK");
            break;
        case 1:
            rsp.setStatus("FAIL");
            break;
        default:
            rsp.setStatus("UNKNOWN");
            break;
    }

    rsp.setReason(can_rsp.reason.c_str());
}
