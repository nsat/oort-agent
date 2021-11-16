#include "catch2/catch.hpp"

#include "Adaptor.h"

TEST_CASE("Adapt AcsMode UAV -> OAPI", "[adaptor][u2o]") {
    // const std::string DecodeAcsMode(const ussp::payload::AcsMode& mode);
    ussp::payload::AcsMode m;
    m.mode = ussp::payload::AcsMode::BDOT;
    SECTION("Decode") {
        std::string decoded = DecodeAcsMode(m);
        REQUIRE(decoded == Adaptor::AcsMode::BDOT);
    }
    SECTION("Adapt") {
        std::string decoded = Adapt(m);
        REQUIRE(decoded == Adaptor::AcsMode::BDOT);
    }
}

TEST_CASE("Adapt QuatT UAV -> OAPI", "[adaptor][u2o]") {
    // const org::openapitools::server::model::Adcs_quat_t Adapt(const ussp::payload::QuatT& src);
    ussp::payload::QuatT q;
    q.q1 = 1.0;
    q.q2 = 2.0;
    q.q3 = 3.0;
    q.q4 = 4.0;
    org::openapitools::server::model::Adcs_quat_t oq = Adapt(q);
    REQUIRE(oq.getQ1() == 1.0);
    REQUIRE(oq.getQ2() == 2.0);
    REQUIRE(oq.getQ3() == 3.0);
    REQUIRE(oq.getQ4() == 4.0);
}

TEST_CASE("Adapt XyzFloat (vector) UAV -> OAPI", "[adaptor][u2o]") {
    // const org::openapitools::server::model::Adcs_xyz_float_t Adapt(const ussp::payload::XyzFloatT& src);
    ussp::payload::XyzFloatT v;
    v.x = 1.0;
    v.y = 2.0;
    v.z = 3.0;
    org::openapitools::server::model::Adcs_xyz_float_t ov = Adapt(v);
    REQUIRE(ov.getX() == 1.0);
    REQUIRE(ov.getY() == 2.0);
    REQUIRE(ov.getZ() == 3.0);
}

TEST_CASE("Adapt Target UAV -> OAPI", "[adaptor][u2o]") {
    // const org::openapitools::server::model::AdcsTarget Adapt(const ussp::payload::TargetT& src);
    ussp::payload::TargetT t;
    t.lat = 3.5;
    t.lon = 2.25;
    org::openapitools::server::model::AdcsTarget ot = Adapt(t);
    REQUIRE(ot.getLat() == 3.5);
    REQUIRE(ot.getLon() == 2.25);
}

TEST_CASE("Adapt CommandResponse UAV -> OAPI", "[adaptor][u2o]") {
    // const org::openapitools::server::model::AdcsCommandResponse
    //      Adapt(const ussp::payload::PayloadAdcsCommand::Response& src);
    ussp::payload::PayloadAdcsCommand::Response r;
    r.status = ussp::payload::PayloadAdcsCommand::Response::STATUS_FAIL;
    r.mode.mode = ussp::payload::AcsMode::NOOP;
    r.reason = "There is no reason, there is only Zuul.";

    SECTION("base") {
        org::openapitools::server::model::AdcsCommandResponse oar = Adapt(r);
        REQUIRE(oar.getMode() == Adaptor::AcsMode::NOOP);
        REQUIRE(oar.getStatus() == Adaptor::Status::FAIL);
        REQUIRE(oar.getReason() == "There is no reason, there is only Zuul.");
        REQUIRE(!oar.quatIsSet());
        REQUIRE(!oar.targetIsSet());
        REQUIRE(!oar.vectorIsSet());
    }

    SECTION("quat") {
        ussp::payload::QuatT q;
        q.q1 = 1; q.q2 = 2; q.q3 = 3; q.q4 = 4;
        r.quat.push_back(q);

        org::openapitools::server::model::AdcsCommandResponse oar = Adapt(r);
        REQUIRE(oar.getMode() == Adaptor::AcsMode::NOOP);
        REQUIRE(oar.getStatus() == Adaptor::Status::FAIL);
        REQUIRE(oar.quatIsSet());
        REQUIRE(oar.getQuat().getQ1() == 1);
        REQUIRE(oar.getQuat().getQ2() == 2);
        REQUIRE(oar.getQuat().getQ3() == 3);
        REQUIRE(oar.getQuat().getQ4() == 4);
    }

    SECTION("target") {
        ussp::payload::TargetT t;
        t.lat = 2.25; t.lon = 3.5;
        r.target.push_back(t);

        org::openapitools::server::model::AdcsCommandResponse oar = Adapt(r);
        REQUIRE(oar.getMode() == Adaptor::AcsMode::NOOP);
        REQUIRE(oar.getStatus() == Adaptor::Status::FAIL);
        REQUIRE(oar.targetIsSet());
        REQUIRE(oar.getTarget().getLat() == 2.25);
        REQUIRE(oar.getTarget().getLon() == 3.5);
    }

    SECTION("vector") {
        ussp::payload::XyzFloatT v;
        v.x = 1; v.y = 2; v.z = 3;
        r.vector.push_back(v);

        org::openapitools::server::model::AdcsCommandResponse oar = Adapt(r);
        REQUIRE(oar.getMode() == Adaptor::AcsMode::NOOP);
        REQUIRE(oar.getStatus() == Adaptor::Status::FAIL);
        REQUIRE(oar.vectorIsSet());
        REQUIRE(oar.getVector().getX() == 1);
        REQUIRE(oar.getVector().getY() == 2);
        REQUIRE(oar.getVector().getZ() == 3);
    }
};

TEST_CASE("Adapt mode OAPI -> UAV", "[adaptor][o2u]") {
    // const uint8_t EncodeAcsMode(const std::string& mode);
    std::string om = Adaptor::AcsMode::NOOP;
    uint8_t um = EncodeAcsMode(om);
    REQUIRE(um == ussp::payload::AcsMode::NOOP);
}

TEST_CASE("Adapt quat OAPI -> UAV", "[adaptor][o2u]") {
    // const ussp::payload::QuatT Adapt(const org::openapitools::server::model::Adcs_quat_t& src); 
    org::openapitools::server::model::Adcs_quat_t oq;
    oq.setQ1(0);
    oq.setQ2(1);
    oq.setQ3(2);
    oq.setQ4(3);
    ussp::payload::QuatT uq = Adapt(oq);
    REQUIRE(uq.q1 == 0);
    REQUIRE(uq.q2 == 1);
    REQUIRE(uq.q3 == 2);
    REQUIRE(uq.q4 == 3);
}

TEST_CASE("Adapt xyzfloat (vector) OAPI -> UAV", "[adaptor][o2u]") {
    // const ussp::payload::XyzFloatT Adapt(const org::openapitools::server::model::Adcs_xyz_float_t& src);
    org::openapitools::server::model::Adcs_xyz_float_t ov;
    ov.setX(5);
    ov.setY(6);
    ov.setZ(7);
    ussp::payload::XyzFloatT uv = Adapt(ov);
    REQUIRE(uv.x == 5);
    REQUIRE(uv.y == 6);
    REQUIRE(uv.z == 7);
}
    
TEST_CASE("Adapt target OAPI -> UAV", "[adaptor][o2u]") {
    // const ussp::payload::TargetT Adapt(const org::openapitools::server::model::AdcsTarget& src);
    org::openapitools::server::model::AdcsTarget ot;
    ot.setLat(3.5);
    ot.setLon(2.25);
    ussp::payload::TargetT ut = Adapt(ot);
    REQUIRE(ut.lat == 3.5);
    REQUIRE(ut.lon == 2.25);
}
    
TEST_CASE("Adapt CommandRequest OAPI -> UAV", "[adaptor][o2u]") {
    // const ussp::payload::PayloadAdcsCommand::Request 
    //      Adapt(const org::openapitools::server::model::AdcsCommandRequest& src);
    org::openapitools::server::model::AdcsCommandRequest oreq;

    oreq.setCommand(Adaptor::AdcsCommand::NADIR);
    oreq.setAperture("GOPRO");

    SECTION("command check") {
        oreq.setCommand(Adaptor::AdcsCommand::NADIR);
        REQUIRE_NOTHROW(Adapt(oreq));
        oreq.setCommand(Adaptor::AdcsCommand::TRACK);
        REQUIRE_NOTHROW(Adapt(oreq));
        oreq.setCommand(Adaptor::AdcsCommand::IDLE);
        REQUIRE_NOTHROW(Adapt(oreq));
        oreq.setCommand("OTHER COMMAND");
        REQUIRE_THROWS(Adapt(oreq));
    }

    SECTION("length check") {
        // max length = 25
        oreq.setAperture("Short enough name no thro");
        REQUIRE_NOTHROW(Adapt(oreq));
        oreq.setAperture("Too long a name so throws.");
        REQUIRE_THROWS(Adapt(oreq));
    }

    SECTION("base") {
        ussp::payload::PayloadAdcsCommand::Request ureq = Adapt(oreq);
        REQUIRE(ureq.adcs_command == ureq.ADCS_COMMAND_NADIR);
        REQUIRE(ureq.aperture == "GOPRO");
        REQUIRE(ureq.angle.size() == 0);
        REQUIRE(ureq.target.size() == 0);
        REQUIRE(ureq.quat.size() == 0);
        REQUIRE(ureq.vector.size() == 0);
    }

    SECTION("angle") {
        oreq.setAngle(13.5);
        ussp::payload::PayloadAdcsCommand::Request ureq = Adapt(oreq);
        REQUIRE(ureq.angle.size() == 1);
        REQUIRE(ureq.angle.front() == 13.5);
    }
    SECTION("target") {
        org::openapitools::server::model::AdcsTarget ot;
        ot.setLat(3.5);
        ot.setLon(3.25);
        oreq.setTarget(ot);
        ussp::payload::PayloadAdcsCommand::Request ureq = Adapt(oreq);
        REQUIRE(ureq.target.size() == 1);
        REQUIRE(ureq.target.front().lat == 3.5);
        REQUIRE(ureq.target.front().lon == 3.25);
    }
    SECTION("quat") {
        org::openapitools::server::model::Adcs_quat_t oq;
        oq.setQ1(4);
        oq.setQ2(3);
        oq.setQ3(2);
        oq.setQ4(1);
        oreq.setQuat(oq);
        ussp::payload::PayloadAdcsCommand::Request ureq = Adapt(oreq);
        REQUIRE(ureq.quat.size() == 1);
        REQUIRE(ureq.quat.front().q1 == 4);
        REQUIRE(ureq.quat.front().q2 == 3);
        REQUIRE(ureq.quat.front().q3 == 2);
        REQUIRE(ureq.quat.front().q4 == 1);
    }
    SECTION("vector") {
        org::openapitools::server::model::Adcs_xyz_float_t ov;
        ov.setX(3.5);
        ov.setY(2.5);
        ov.setZ(1.5);
        oreq.setVector(ov);
        ussp::payload::PayloadAdcsCommand::Request ureq = Adapt(oreq);
        REQUIRE(ureq.vector.size() == 1);
        REQUIRE(ureq.vector.front().x == 3.5);
        REQUIRE(ureq.vector.front().y == 2.5);
        REQUIRE(ureq.vector.front().z == 1.5);
    }
}

TEST_CASE("Adapt double", "[adaptor][generic]") {
    // const double Adapt(const double src);
    double s = 3.14159;
    double t = Adapt(s);
    REQUIRE(t == 3.14159);
}

TEST_CASE("Adapt float", "[adaptor][generic]") {
    // const float Adapt(const float src);
    float s = 1.5 ;
    float t = Adapt(s);
    REQUIRE(t == 1.5 );
}

TEST_CASE("Adapt int", "[adaptor][generic]") {
    // const int Adapt(const int src);
    int s = 42;
    int t = Adapt(s);
    REQUIRE (t == 42);
}

TEST_CASE("Adapt uint", "[adaptor][generic]") {
    // const unsigned int Adapt(const unsigned int src);
    unsigned int s = 99;
    unsigned int t = Adapt(s);
    REQUIRE(t == 99);
};

    
