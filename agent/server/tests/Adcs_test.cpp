#include <thread>

#include "catch2/catch.hpp"

#include "Adcs.h"

using namespace Catch::Matchers;

TEST_CASE("receive values", "[adcs]") {
    AdcsManager mgr;

    ussp::payload::PayloadAdcsFeed msg;

    msg.acs_mode_active.mode = msg.acs_mode_active.NOOP;
    msg.r_eci.x = 0.0 ;
    msg.r_eci.y = 1.0e7 ;
    msg.r_eci.z = 1.0e7 ;

    mgr.setAdcs(msg);
    auto result = mgr.getAdcs();

    REQUIRE( result.getHk().getAcsModeActive() == "NO-OP" );
    REQUIRE( result.getHk().getREci().getY() == 1e7 );
    REQUIRE_THAT( result.getHk().getLatDeg(), WithinRel(45.0) );
}

struct testdata {
    double unix_timestamp;
    float eci_x, eci_y, eci_z;
    float error_q1, error_q2, error_q3, error_q4;
    float qbo_q1, qbo_q2, qbo_q3, qbo_q4;

    float ecef_x, ecef_y, ecef_z;
    float lat, lon, alt;
    float error_angle;
    float pitch, roll, yaw;
};

TEST_CASE("ECEF math", "[adcs]") {
    AdcsManager mgr;

    ussp::payload::PayloadAdcsFeed msg;

    // random snag from telemetry - see adcs_telem_sample.txt
    // note that the recorded derived telemetry values suggest significantly
    // more accuracy than they in fact provide.  The raw data is primarily
    // delivered as floats, but python does all the math in doubles.
    auto cases = GENERATE(
        testdata {
            .unix_timestamp = 1636051202.13,
            .eci_x = -6825570, .eci_y = -945956.7, .eci_z = 560650.2,
            .error_q1 = 0.0058, .error_q2 = 0.000248, .error_q3 = 0.015588, .error_q4 = 0.999862,
            .qbo_q1 = 0.000735, .qbo_q2 = 0.005759, .qbo_q3 = 0.988149, .qbo_q4 = 0.153391,
            .ecef_x = -4985785.303578716, .ecef_y = -4756593.82434462, .ecef_z = 560650.2,
            .lat = 4.65145547471229, .lon = -136.347646460456, .alt = 542578.563228665,
            .error_angle = 1.90375985105757,
            .pitch = 0.018001102818259736, .roll = 0.6650455840072211, .yaw = 162.35283026482037
        },
        testdata {
            .unix_timestamp = 1636051202.87,
            .eci_x = 44745.74, .eci_y = 3433632, .eci_z = -6006706,
            .error_q1 = 0.019702, .error_q2 = 0.030384, .error_q3 = -0.002096, .error_q4 = 0.999342,
            .qbo_q1 = 0.01603, .qbo_q2 = 0.032472, .qbo_q3 = 0.114281, .qbo_q4 = 0.992788,
            .ecef_x = -1970219.599835531, .ecef_y = 2812483.8881487064, .ecef_z = -6006706,
            .lat = -60.2441892697613,  .lon = 125.012262421601, .alt = 547984.597692652,
            .error_angle = 4.15723322355399,
            .pitch = 3.486409140687266, .roll = 2.253648448177422, .yaw = 13.201563366584798
        }
    );

    msg.unix_timestamp = cases.unix_timestamp ;

    msg.r_eci .x = cases.eci_x;
    msg.r_eci .y = cases.eci_y;
    msg.r_eci .z = cases.eci_z;

    msg.control_error_q .q1 = cases.error_q1;
    msg.control_error_q .q2 = cases.error_q2;
    msg.control_error_q .q3 = cases.error_q3;
    msg.control_error_q .q4 = cases.error_q4;

    msg.q_bo_est .q1 = cases.qbo_q1;
    msg.q_bo_est .q2 = cases.qbo_q2;
    msg.q_bo_est .q3 = cases.qbo_q3;
    msg.q_bo_est .q4 = cases.qbo_q4;
    // derivations:
    // eci + time => ecef, lat, lon, altitude
    // control_error_q => control error angle
    // q_bo_est => euler angles

    mgr.setAdcs(msg);
    auto result = mgr.getAdcs();

    CHECK_THAT( result.getHk().getREcef().getX(), WithinRel(cases.ecef_x)); 
    CHECK_THAT( result.getHk().getREcef().getY(), WithinRel(cases.ecef_y)); 
    CHECK_THAT( result.getHk().getREcef().getZ(), WithinRel(cases.ecef_z));
    
    CHECK_THAT( result.getHk().getLatDeg(), WithinRel(cases.lat));
    CHECK_THAT( result.getHk().getLonDeg(), WithinRel(cases.lon));
    CHECK_THAT( result.getHk().getAltitude(), WithinRel(cases.alt));

    CHECK_THAT( result.getHk().getControlErrorAngleDeg(), WithinRel(cases.error_angle, 1e-4f));
    
    CHECK_THAT( result.getHk().getEulerAngles().getPitch(), WithinRel(cases.pitch));
    CHECK_THAT( result.getHk().getEulerAngles().getRoll(), WithinRel(cases.roll));
    CHECK_THAT( result.getHk().getEulerAngles().getYaw(), WithinRel(cases.yaw));
}

TEST_CASE("TFRS cache", "[adcs]") {
    AdcsManager mgr;

    ussp::tfrs::ReceiverNavigationState msg;

    msg.ecef_pos_x = 1.0;
    msg.ecef_pos_y = 2.0;
    msg.ecef_pos_z = 3.0;

    mgr.setTfrs(msg);
    auto result = mgr.getTfrs();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto result2 = mgr.getTfrs();

    // second result should be identical to the first, except for the age
    // should go up
    REQUIRE( result.getAge() < result2.getAge());
    REQUIRE( result.getEcefPosX() == result2.getEcefPosX());
}
