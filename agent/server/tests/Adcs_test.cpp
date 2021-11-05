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

TEST_CASE("ECEF math", "[adcs]") {
    AdcsManager mgr;

    ussp::payload::PayloadAdcsFeed msg;

    // random snag from telemetry
    /*
 "r_ecef": {
    "x": -438248.11324512196,
    "y": -760559.6205128727,
    "z": 6801784
  },
  "r_eci": {
    "x": 95803.609375,
    "y": -872544.5625,
    "z": 6801784
  },
  "unix_timestamp": 1635896101.78,
  "lon_deg": -119.95131875423876,
  "lat_deg": 82.64647077564702,
  "control_error_q": {
    "q1": 0.027088,
    "q2": -0.013493,
    "q3": 0.052024,
    "q4": 0.998187
  }
  */

    msg.r_eci.x = 95803.609375;
    msg.r_eci.y = -872544.5625;
    msg.r_eci.z = 6801784;
    msg.unix_timestamp = 1635896101.78;
    msg.control_error_q.q1 = 0.027088;
    msg.control_error_q.q2 = -0.013493;
    msg.control_error_q.q3 = 0.052024;
    msg.control_error_q.q4 = 0.998187;

    mgr.setAdcs(msg);
    auto result = mgr.getAdcs();

    CHECK( result.getHk().getREcef().getX() == -438248.11324512196);
    CHECK( result.getHk().getREcef().getY() == -760559.6205128727);
    CHECK( result.getHk().getREcef().getZ() == 6801784);
    CHECK_THAT( result.getHk().getLonDeg(), WithinRel(-119.95131875423876));
    CHECK_THAT( result.getHk().getLatDeg(), WithinRel(82.64647077564702));
    CHECK_THAT( result.getHk().getControlErrorQ().getQ4(), WithinRel(0.998187f));
    CHECK_THAT( result.getHk().getControlErrorAngleDeg(), WithinRel(6.9013199875592015f));
}
