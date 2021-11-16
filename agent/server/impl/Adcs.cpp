/**
 * Adcs.cpp
 * 
 * Adcs and Tfrs conversion and caching utility.
 * 
 * Copyright (c) 2021 Spire Global, Inc.
 */
#include "Adcs.h"
#include "Adaptor.h"
#include <cmath>
#include <string>

using namespace std;
using namespace org::openapitools::server::model;

const double R_EARTH = 6371000.0;

static double rad2deg(double a) {
    return 180.0 / M_PI * a;
}

static double acos_safe(double a) {
    if (fabs(a) > 1.0) {
        return acos(copysign(1.0, a));
    } else {
        return acos(a);
    }
}

static double asin_safe(double a) {
    if (fabs(a) > 1.0) {
        return asin(copysign(1.0, a));
    } else {
        return asin(a);
    }
}

/**
 * \brief Convert unix epoch timestamp to Greenwich Hour Angle
 * 
 * \param unix_timestamp 
 * \return double 
 */
static double get_gha(const double unix_timestamp) {
    double jd = unix_timestamp / 86400 + 2440587.5;
    const double C1 = 1.72027916940703639e-2;
    const double C1P2P = C1 + 2 * M_PI;
    const double THGR70 = 1.7321343856509374;
    const double FK5R = 5.07551419432269442e-15;
    const double days50 = jd - 2400000.5 - 33281.0;
    const double ts70 = days50 - 7305.0;
    const int ids70 = static_cast<int>(ts70 + 1.0e-8);
    const double ds70 = static_cast<double>(ids70);
    const double trfac = ts70 - ds70;
    double theta = THGR70 + C1*ds70 + C1P2P*trfac + ts70*ts70*FK5R;
    theta = fmod(theta, 2 * M_PI);
    if (theta < 0.0) {
        theta += 2 * M_PI;
    }
    double gha = theta;
    return gha;
}

const double xyz_norm(const ussp::payload::XyzFloatT& v) {
    return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}

const double xyz_norm(const org::openapitools::server::model::Adcs_xyz_float_t& v) {
    return sqrt(pow(v.getX(), 2) + pow(v.getY(), 2) + pow(v.getZ(), 2));
}

static double DeriveControlErrorAngleDeg(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    return 2.0 * rad2deg(acos_safe(ucan_adcs.control_error_q.q4));
}

static org::openapitools::server::model::Adcs_euler_t DeriveEulerAngles(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    org::openapitools::server::model::Adcs_euler_t value;
    auto q = ucan_adcs.q_bo_est;
    const double dcm_00 = pow(q.q4, 2) + pow(q.q1, 2) - pow(q.q2, 2) - pow(q.q3, 2);
    const double dcm_01 = 2.0 * (q.q1 * q.q2 + q.q3 * q.q4);
    const double dcm_02 = 2.0 * (q.q1 * q.q3 - q.q2 * q.q4);
    const double dcm_12 = 2.0 * (q.q2 * q.q3 + q.q1 * q.q4);
    const double dcm_22 = pow(q.q4, 2) - pow(q.q1, 2) - pow(q.q2, 2) + pow(q.q3, 2);
    value.setRoll(rad2deg(atan2(dcm_12, dcm_22)));
    value.setPitch(rad2deg(asin_safe(-dcm_02)));
    value.setYaw(rad2deg(atan2(dcm_01, dcm_00)));
    return value;
}

static double DeriveLatDeg(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    auto r_ecef = oapi_hk.getREcef();
    double r_sat = xyz_norm(r_ecef);
    if (r_sat > R_EARTH) {
        return rad2deg(asin_safe(r_ecef.getZ() / r_sat));
    } else {
        return 0.0;
    }
}

static double DeriveLonDeg(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    auto r_ecef = oapi_hk.getREcef();
    double r_sat = xyz_norm(r_ecef);
    if (r_sat > R_EARTH) {
        return rad2deg(atan2(r_ecef.getY() / r_sat, r_ecef.getX() / r_sat));
    } else {
        return 0.0;
    }
}

static double DeriveAltitude(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    double r_sat = xyz_norm(oapi_hk.getREcef());
    if (r_sat > R_EARTH) {
        return r_sat - R_EARTH;
    } else {
        return 0.0;
    }
}

static org::openapitools::server::model::Adcs_xyz_float_t DeriveREcef(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
    org::openapitools::server::model::Adcs_xyz_float_t ecef;
    const double gha = get_gha(ucan_adcs.unix_timestamp);
    ecef.setX(cos(gha) * ucan_adcs.r_eci.x + sin(gha) * ucan_adcs.r_eci.y);
    ecef.setY(-sin(gha) * ucan_adcs.r_eci.x + cos(gha) * ucan_adcs.r_eci.y);
    ecef.setZ(ucan_adcs.r_eci.z);
    return ecef;
}

static void convert(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {

    oapi_hk.setAcsModeActive(Adapt(ucan_adcs.acs_mode_active));
    oapi_hk.setControlErrorQ(Adapt(ucan_adcs.control_error_q));
    oapi_hk.setQBoEst(Adapt(ucan_adcs.q_bo_est));
    oapi_hk.setLatlontrackLat(Adapt(ucan_adcs.latlontrack_lat));
    oapi_hk.setQBiEst(Adapt(ucan_adcs.q_bi_est));
    oapi_hk.setLatlontrackLon(Adapt(ucan_adcs.latlontrack_lon));
    oapi_hk.setREci(Adapt(ucan_adcs.r_eci));
    oapi_hk.setLatlontrackBodyVector(Adapt(ucan_adcs.latlontrack_body_vector));
    oapi_hk.setOmegaBoEst(Adapt(ucan_adcs.omega_bo_est));
    oapi_hk.setAcsModeCmd(Adapt(ucan_adcs.acs_mode_cmd));
    oapi_hk.setVEci(Adapt(ucan_adcs.v_eci));
    oapi_hk.setQcf(Adapt(ucan_adcs.qcf));
    oapi_hk.setLeaseTimeRemaining(Adapt(ucan_adcs.lease_time_remaining));
    oapi_hk.setUnixTimestamp(Adapt(ucan_adcs.unix_timestamp));
    oapi_hk.setOmegaBiEst(Adapt(ucan_adcs.omega_bi_est));
    oapi_hk.setControlErrorOmega(Adapt(ucan_adcs.control_error_omega));
    oapi_hk.setEclipseFlag(Adapt(ucan_adcs.eclipse_flag));
    oapi_hk.setLeaseActive(Adapt(ucan_adcs.lease_active));

    oapi_hk.setREcef(DeriveREcef(oapi_hk, ucan_adcs));
    oapi_hk.setControlErrorAngleDeg(DeriveControlErrorAngleDeg(oapi_hk, ucan_adcs));
    oapi_hk.setEulerAngles(DeriveEulerAngles(oapi_hk, ucan_adcs));
    oapi_hk.setLatDeg(DeriveLatDeg(oapi_hk, ucan_adcs));
    oapi_hk.setLonDeg(DeriveLonDeg(oapi_hk, ucan_adcs));
    oapi_hk.setAltitude(DeriveAltitude(oapi_hk, ucan_adcs));
}

void AdcsManager::setTfrs(const ussp::tfrs::ReceiverNavigationState& tfrs)  {
    m_tfrs.setUtcTime(tfrs.utc_time);
    m_tfrs.setEcefPosX(tfrs.ecef_pos_x);
    m_tfrs.setEcefPosY(tfrs.ecef_pos_y);
    m_tfrs.setEcefPosZ(tfrs.ecef_pos_z);
    m_tfrs.setEcefVelX(tfrs.ecef_vel_x);
    m_tfrs.setEcefVelY(tfrs.ecef_vel_y);
    m_tfrs.setEcefVelZ(tfrs.ecef_vel_z);
    m_tfrs.setAge(0);
    m_tfrs_mtime = chrono::system_clock::now();
}

TfrsResponse AdcsManager::getTfrs() {
    chrono::duration<double> since = chrono::system_clock::now() - m_tfrs_mtime;
    m_tfrs.setAge(since.count());
    return m_tfrs;
}

void AdcsManager::setAdcs(const ussp::payload::PayloadAdcsFeed& adcs) {
    AdcsHk hk = m_adcs.getHk();
    convert(hk, adcs);
    m_adcs.setMode(hk.getAcsModeActive());
    m_adcs.setHk(hk);
    m_adcs.setAge(0);
    m_adcs_mtime = chrono::system_clock::now();
}

AdcsResponse AdcsManager::getAdcs() {
    chrono::duration<double> since = chrono::system_clock::now() - m_adcs_mtime;
    m_adcs.setAge(since.count());
    return m_adcs;
}
