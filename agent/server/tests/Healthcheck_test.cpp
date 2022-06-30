#include "catch2/catch.hpp"

#include <unistd.h>
#include <fstream>

#include "AgentUAVCANServer.h"
#include "Log.h"

#include <uavcan_linux/uavcan_linux.hpp>
#include <uavcan/helpers/ostream.hpp>
#include <ussp/payload/PayloadHealthCheck.hpp>

#define REQUIRE_VALID_OUTPUT() do { \
        REQUIRE(rsp.timestamp == 1628760852); \
        REQUIRE(rsp.need_hard_reset == false); \
        REQUIRE(rsp.schema_version == 1); \
        REQUIRE(rsp.imetric.size() == 3); \
        REQUIRE(rsp.fmetric.size() == 4); \
        REQUIRE(rsp.imetric_feeds.size() == 3); \
        REQUIRE(rsp.fmetric_feeds.size() == 4); \
        REQUIRE(rsp.imetric[0] == 1); \
        REQUIRE(rsp.imetric[1] == 2); \
        REQUIRE(rsp.imetric[2] == 3); \
        REQUIRE(rsp.fmetric[0] == Approx(0.2)); \
        REQUIRE(rsp.fmetric[1] == Approx(0.3)); \
        REQUIRE(rsp.fmetric[2] == Approx(0.4)); \
        REQUIRE(rsp.fmetric[3] == Approx(2.86)); \
        REQUIRE(rsp.imetric_feeds[0] == 4); \
        REQUIRE(rsp.imetric_feeds[1] == 5); \
        REQUIRE(rsp.imetric_feeds[2] == 6); \
        REQUIRE(rsp.fmetric_feeds[0] == 10); \
        REQUIRE(rsp.fmetric_feeds[1] == 11); \
        REQUIRE(rsp.fmetric_feeds[2] == 24); \
        REQUIRE(rsp.fmetric_feeds[3] == 32); \
    } while(0)

using namespace std;
//using Catch::Matchers::StartsWith;

template <typename DataStruct>
struct ReceivedDataStructureSpec : public uavcan::ReceivedDataStructure<DataStruct>
{
    ReceivedDataStructureSpec() : uavcan::ReceivedDataStructure<DataStruct>(UAVCAN_NULLPTR) { }
};

TEST_CASE("healthcheck response") {

    Log::setLevel(Log::Info);
    Log::setOut(cout);

    AgentUAVCANServer server("can0", 101);

    SECTION("successful output") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_success.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 0);
        REQUIRE(rsp.health_state == 0);
        REQUIRE_VALID_OUTPUT();
    }

    SECTION("error status") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_error.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 0);
        REQUIRE(rsp.health_state == 122);
        REQUIRE_VALID_OUTPUT();
    }

    SECTION("invalid json") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_invalid_json.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 22);
        REQUIRE(rsp.health_state == 0);
    }

    SECTION("nonexistent payload_healthcheck") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./foo/bar/baz/payload_healthcheck");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 22);
        REQUIRE(rsp.health_state == 127);
    }

    SECTION("too much data") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_long_response.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 22);
        REQUIRE(rsp.health_state == 137);
    }

    SECTION("timeout") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_hanging.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 0);
        REQUIRE(rsp.health_state == 137); // Because of the SIGKILL
        REQUIRE_VALID_OUTPUT(); // Timeout shouldn't affect that we read a valid output before timing out
    }

    SECTION("timeout, again") {
        // Should confirm that a SIGCHLD from the previous section wasn't left pending
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_hanging.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 0);
        REQUIRE(rsp.health_state == 137); // Because of the SIGKILL
        REQUIRE_VALID_OUTPUT(); // Timeout shouldn't affect that we read a valid output before timing out
    }

    SECTION("invalid JSON field type") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_invalid_int_metrics.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 50);
        REQUIRE(rsp.health_state == 0);
    }

    SECTION("JSON array is too long") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_long_array.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 50);
        REQUIRE(rsp.health_state == 0);
    }

    SECTION("garbage binary characters") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_garbage.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 22);
        REQUIRE(rsp.health_state == 0);
    }

    SECTION("trailing garbage after JSON") {
        ReceivedDataStructureSpec<ussp::payload::PayloadHealthCheck::Request> req;
        ussp::payload::PayloadHealthCheck::Response rsp;
        server.setPayloadHealthcheckCommand("./build/server/tests/utils/hk_trailing_garbage.sh");
        server.healthCheckHandler(req, rsp);
        REQUIRE(rsp.response_code == 22);
        REQUIRE(rsp.health_state == 0);
    }
}
