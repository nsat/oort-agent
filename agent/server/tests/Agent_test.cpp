#include <unistd.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "Config.h"
#include "Agent.h"
#include "Log.h"
#include "AgentUAVCANServer.h"
#include "Adaptor.h"

// Note, Catch2 defines conflict with uavcan defines, so include this last
#include "catch2/catch.hpp"

using namespace std;
using Catch::Matchers::StartsWith;
using Catch::Matchers::Contains;

class SecretAgent {
 public:
  void clear_metacache(Agent& a) {
    a.metafile_cache.flush();
  }
  void clear_dircache(Agent& a) {
    a.dir_cache.flush();
  }
  void disable_caches(Agent& a) {
    a.dir_cache.disable();
    a.metafile_cache.disable();
  }
};

TEST_CASE("setup, send -> [fake collect/transfer/deliver] -> query -> retrieve",
          "[agent][api]") {

    stringstream dummy_out;
    Log::setLevel(Log::Info);
    Log::setOut(dummy_out);

    AgentConfig cfg;
    char *argv[] = {strdup("UNITTEST"), strdup("-w"), NULL};
    int argc = (sizeof(argv) / sizeof(argv[0]));
    SECTION("bad directory") {
        argv[2] = strdup("/asdf");
        REQUIRE(cfg.parseOptions(argc, argv) == false);
    }

    SECTION("setup ok") {
        char worktmpl[] = "/tmp/unittest_agentXXXXXX";
        char *dtmp = mkdtemp(worktmpl);
        argv[2] = dtmp;

        REQUIRE(cfg.parseOptions(argc, argv) == true);
        Agent a(cfg);
        SecretAgent().disable_caches(a);
        REQUIRE(a.getWorkdir() == dtmp);

        InfoRequest req;
        auto resp = a.collector_info(req);
        SystemInfo si = resp.result.getSysinfo();
        INFO("validate setup");
        REQUIRE(si.getUploads() == string(dtmp) + "/uploads");
        REQUIRE(si.getAgentUpgrades() == string(dtmp) + "/upgrades");
        REQUIRE(si.getWorkdir() == string(dtmp) + "/transfers");
        REQUIRE(si.getDead() == string(dtmp) + "/dead");

        INFO("topics");
        REQUIRE(resp.result.getTopics().empty());

        req.setTopics(vector<string>({"test"}));
        resp = a.collector_info(req);
        REQUIRE(!resp.result.getTopics().empty());


        INFO("send_file");
        char clitmpl[] = "/tmp/unittest_clientXXXXXX";
        char *ctmp = mkdtemp(clitmpl);

        // create temporary files in the test client directory
        auto tmpf = [&](const char *d = "The quick brown fox jumps over the lazy dog") {
            string ftmpl(ctmp);
            ftmpl.append("/sendfileXXXXXX");
            char *ftmp = strdup(ftmpl.c_str());

            FILE *f = fdopen(mkstemp(ftmp), "wb");
            // write some data to it
            fputs(d, f);
            fclose(f);
            return ftmp;
        };
        char *ftmps[2] = {tmpf(), tmpf()};

        // call send_file
        SendFileRequest sf_req;
        sf_req.setDestination("ground");
        sf_req.setFilepath(tmpf());
        sf_req.setTopic("test-fail");

        auto sf_resp = a.send_file(sf_req);
        // verify that file+data+meta are in transfer directory
        REQUIRE(sf_resp.code == Code::Bad_Request);
        REQUIRE_THAT(sf_resp.err.getMessage(), StartsWith("Topic "));

        sf_req.setTopic("test");
        sf_resp = a.send_file(sf_req);
        REQUIRE(sf_resp.code == Code::Ok);

        // send a second file
        sf_req.setFilepath(tmpf());
        auto sf_resp2 = a.send_file(sf_req);
        REQUIRE(sf_resp.code == Code::Ok);

        string transfer_filename(si.getWorkdir());
        transfer_filename.append("/")
          .append(sf_resp.result.getUUID())
          .append(".data.oort");

        WARN(transfer_filename);
        REQUIRE(access(transfer_filename.c_str(), F_OK) == 0);

        INFO("query_available");

        // nothing here yet
        auto qa_resp_empty = a.query_available("test");
        REQUIRE(qa_resp_empty.result.getFiles().size() == 0);

        // move "send_file" created files from trenasfer to uploads
        // directory to test retrieve functions
        auto mvId = [si](ResponseCode<SendFileResponse> resp) {
          rename(
               si.getWorkdir().append("/").append(resp.result.getUUID().append(".data.oort")).c_str(),
               si.getUploads().append("/").append(resp.result.getUUID().append(".data.oort")).c_str()
          );
          rename(
               si.getWorkdir().append("/").append(resp.result.getUUID()).append(".meta.oort").c_str(),
               si.getUploads().append("/").append(resp.result.getUUID()).append(".meta.oort").c_str()
          );
        };

        mvId(sf_resp);
        // call query_available
        auto qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);

        // create a junk (non-js) meta file.
        INFO("bad meta file");
        ofstream badfile(si.getUploads().append("/abcd.meta.oort"));
        badfile << "bad file" << endl;
        badfile.close();

        // no dead files yet (hasn't checked)
        resp = a.collector_info(req);
        REQUIRE(resp.result.getAvailable().getDead().size() == 0);

        // still works
        qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);

        // now a dead file
        resp = a.collector_info(req);
        REQUIRE(resp.result.getAvailable().getDead().size() == 1);

        // create a parsable but non-schema meta file.
        INFO("non-schema meta file");
        ofstream badjsfile(si.getUploads().append("/badjs.meta.oort"));
        badjsfile << "{\"destination\":\"ground\"}" << endl;
        badjsfile.close();

        // still works
        qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);

        // now a dead file
        resp = a.collector_info(req);
        REQUIRE(resp.result.getAvailable().getDead().size() == 2);

        // move second "send_file" created files over to uploads directory
        mvId(sf_resp2);

        // call query_available
        qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 2);
        REQUIRE(qa_resp.result.overflowIsSet() == false);

        INFO("max query");
        a.setMaxQuery(1);
        qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);
        REQUIRE(qa_resp.result.isOverflow() == true);

        // verify file results
        // TODO(jeff.rogers)

        INFO("retrieve_file");
        // files from send_file in uploads directory
        // get id from query_available
        RetrieveFileRequest rf_req;
        rf_req.setId(qa_resp.result.getFiles().front().getId());
        string save_path(ctmp);
        save_path.append("/destfile");
        rf_req.setSavePath(save_path);

        // note: file in transfers
        auto upload_file = si.getUploads().append("/").append(rf_req.getId()).append(".data.oort");
        INFO("upload file exists");
        REQUIRE(access(upload_file.c_str(), F_OK) == 0);

        // call retrieve_file transferring to temporary directory
        auto rf_resp = a.retrieve_file(rf_req);

        // verify file in new directory, deleted from uploads
        INFO("file saved");
        REQUIRE(access(save_path.c_str(), F_OK) == 0);
        INFO("upload file removed");
        REQUIRE(access(upload_file.c_str(), F_OK) != 0);

        // verify only 1 file remaining from query_available
        auto qa_resp_post = a.query_available("test");
        REQUIRE(qa_resp_post.result.getFiles().size() == 1);
        REQUIRE(qa_resp_post.result.overflowIsSet() == false);

        SECTION("corrupted data before query") {
          // let's corrupt that data file now
          auto corrupt_file = si.getUploads().append("/").append(
               qa_resp_post.result.getFiles()[0].getId()).append(".data.oort");
          fstream corruption(corrupt_file);
          corruption << "X";
          corruption.close();

          // 2 dead files at this point (non-json meta, bad json meta)
          // (corruption hasn't been detected yet)
          REQUIRE(a.collector_info(req).result.getAvailable().getDead().size() == 2);

          // verify no files from query_available
          qa_resp_post = a.query_available("test");
          REQUIRE(qa_resp_post.result.getFiles().size() == 0);

          // verify now 4 files (non-json meta, original bad json meta, the corrupted data and meta)
          resp = a.collector_info(req);
          REQUIRE(resp.result.getAvailable().getDead().size() == 4);
        }
        SECTION("corrupted data after query / before retrieve") {
          // let's corrupt that data file now
          auto corrupt_file = si.getUploads().append("/").append(
               qa_resp_post.result.getFiles()[0].getId()).append(".data.oort");

          // verify files from query_available
          qa_resp_post = a.query_available("test");
          REQUIRE(qa_resp_post.result.getFiles().size() == 1);
          // 2 dead files at this point (non-json meta, bad json meta)
          REQUIRE(a.collector_info(req).result.getAvailable().getDead().size() == 2);

          fstream corruption(corrupt_file);
          corruption << "X";
          corruption.close();

          rf_req.setId(qa_resp_post.result.getFiles().front().getId());
          string save_path(ctmp);
          save_path.append("/destfile2");
          rf_req.setSavePath(save_path);

          auto rf_resp = a.retrieve_file(rf_req);
          REQUIRE(rf_resp.code == Code::Bad_Request);

          // verify now 4 files (non-json meta, original bad json meta, the corrupted data and meta)
          REQUIRE(a.collector_info(req).result.getAvailable().getDead().size() == 4);
        }
        SECTION("removed meta before query") {
          REQUIRE(a.query_available("test").result.getFiles().size() == 1);

          auto gone_file = si.getUploads().append("/").append(
               qa_resp_post.result.getFiles()[0].getId()).append(".data.oort");
          REQUIRE(unlink(gone_file.c_str()) == 0);

          // verify no files from query_available
          qa_resp_post = a.query_available("test");
          REQUIRE(qa_resp_post.result.getFiles().size() == 0);

          // verify now 4 files (non-json meta, original bad json meta, the
          // corrupted data (no meta)
          resp = a.collector_info(req);
          REQUIRE(resp.result.getAvailable().getDead().size() == 3);
        }


        
        INFO("cleanup");
        // cleanup
        rmdir(si.getUploads().c_str());
        rmdir(si.getAgentUpgrades().c_str());
        rmdir(si.getWorkdir().c_str());
        rmdir(dtmp);
    }
}

TEST_CASE("adcs/tfrs get", "[!hide][adcs-integration]") {
    AgentConfig cfg;
    char *argv[] = {strdup("UNITTEST"),
        strdup("-w"), NULL,
        strdup("-c"), strdup("can0"),
        strdup("-n"), strdup("12"),
        strdup("-N"), strdup("51")
    };
    int argc = (sizeof(argv) / sizeof(argv[0]));

    char worktmpl[] = "/tmp/unittest_agentXXXXXX";
    char *dtmp = mkdtemp(worktmpl);
    argv[2] = dtmp;

    AdcsManager mgr;

    REQUIRE(cfg.parseOptions(argc, argv) == true);

    stringstream dummy_out;
    Log::setLevel(Log::Info);
    Log::setOut(dummy_out);

    Agent a(cfg);

    SECTION("tfrs get disabled") {
      auto resp = a.tfrs_get();
      REQUIRE(resp.code == Code::Bad_Request);
      REQUIRE_THAT(resp.err.getMessage(), Contains("unavailable"));
    }
    SECTION("adcs get disabled") {
      auto resp = a.adcs_get();
      REQUIRE(resp.code == Code::Bad_Request);
      REQUIRE_THAT(resp.err.getMessage(), Contains("unavailable"));
    }

    INFO("Starting UAVCAN server");
    auto can_server = new AgentUAVCANServer(
      cfg.getCANInterface(), cfg.getUAVCANNodeID());
    can_server->start();
    a.setAdcsMgr(&can_server->m_mgr);

    SECTION("wait for feed") {
      std::this_thread::sleep_for(chrono::seconds(5));
      SECTION("tfrs get ok") {
        auto resp = a.tfrs_get();
        REQUIRE(resp.code == Code::Ok);
        REQUIRE(resp.result.getAge() < 5);
        CHECK(resp.result.getEcefPosX() == 2.5);
        CHECK(resp.result.getEcefPosY() == 3.5);
        CHECK(resp.result.getEcefPosZ() == 4.5);
        CHECK(resp.result.getEcefVelX() == 5.5);
        CHECK(resp.result.getEcefVelY() == 6.5);
        CHECK(resp.result.getEcefVelZ() == 7.5);
        CHECK(resp.result.getUtcTime() == 1234);
      }
      SECTION("tfrs get cached") {
        auto resp = a.tfrs_get();
        REQUIRE(resp.code == Code::Ok);
        // check caching.  COULD FAIL IF NEW MESSAGE COMES IN DURING SLEEP
        std::this_thread::sleep_for(chrono::milliseconds(10));
        auto resp2 = a.tfrs_get();
        REQUIRE(resp2.result.getAge() > resp.result.getAge());
      }
      SECTION("adcs get ok") {
        auto resp = a.adcs_get();
        REQUIRE(resp.code == Code::Ok);
        CHECK(resp.result.getMode() == Adaptor::AcsMode::SUNSPIN);
        // CHECK(resp.result.getController() == "?????");  // not mplemented?
        // CHECK(resp.result.getAge() == 0); // implemented??
        auto hk = resp.result.getHk();
        CHECK(hk.getAcsModeActive() == Adaptor::AcsMode::SUNSPIN);
        CHECK(hk.getAcsModeCmd() == Adaptor::AcsMode::NADIRPOINTYAW);

        CHECK(hk.getControlErrorQ().getQ1() == 1);
        CHECK(hk.getControlErrorQ().getQ2() == 2);
        CHECK(hk.getControlErrorQ().getQ3() == 3);
        CHECK(hk.getControlErrorQ().getQ4() == 4);

        CHECK(hk.getQBoEst().getQ1() == 5);
        CHECK(hk.getQBoEst().getQ2() == 6);
        CHECK(hk.getQBoEst().getQ3() == 7);
        CHECK(hk.getQBoEst().getQ4() == 8);

        CHECK(hk.getLatlontrackLat() == 99);
        CHECK(hk.getLatlontrackLon() == 88);

        CHECK(hk.getLeaseActive());
        CHECK(hk.getEclipseFlag());

        CHECK(hk.getQBiEst().getQ1() == 9);
        CHECK(hk.getQBiEst().getQ2() == 10);
        CHECK(hk.getQBiEst().getQ3() == 11);
        CHECK(hk.getQBiEst().getQ4() == 12);

        CHECK(hk.getREci().getX() == -1);
        CHECK(hk.getREci().getY() == -2);
        CHECK(hk.getREci().getZ() == -3);

        CHECK(hk.getLatlontrackBodyVector().getX() == -4);
        CHECK(hk.getLatlontrackBodyVector().getY() == -5);
        CHECK(hk.getLatlontrackBodyVector().getZ() == -6);

        CHECK(hk.getOmegaBoEst().getX() == -7);
        CHECK(hk.getOmegaBoEst().getY() == -8);
        CHECK(hk.getOmegaBoEst().getZ() == -9);

        CHECK(hk.getVEci().getX() == -11);
        CHECK(hk.getVEci().getY() == -12);
        CHECK(hk.getVEci().getZ() == -13);

        CHECK(hk.getQcf().getQ1() == 21);
        CHECK(hk.getQcf().getQ2() == 22);
        CHECK(hk.getQcf().getQ3() == 23);
        CHECK(hk.getQcf().getQ4() == 24);

        CHECK(hk.getLeaseTimeRemaining() == 3600);
        CHECK(hk.getUnixTimestamp() == 1234.5);

        CHECK(hk.getOmegaBiEst().getX() == -31);
        CHECK(hk.getOmegaBiEst().getY() == -32);
        CHECK(hk.getOmegaBiEst().getZ() == -33);

        CHECK(hk.getControlErrorOmega().getX() == -41);
        CHECK(hk.getControlErrorOmega().getY() == -42);
        CHECK(hk.getControlErrorOmega().getZ() == -43);
      }
      SECTION("adcs get cached") {
        auto resp = a.adcs_get();
        REQUIRE(resp.code == Code::Ok);
        // check caching.  COULD FAIL IF NEW MESSAGE COMES IN DURING SLEEP
        std::this_thread::sleep_for(chrono::milliseconds(10));
        auto resp2 = a.adcs_get();
        REQUIRE(resp2.result.getAge() > resp.result.getAge());
      }
    }

    SECTION("adcs command disabled") {
      auto cmd = AdcsCommandRequest();
      auto resp = a.adcs_command(cmd);
      REQUIRE(resp.code == Code::Bad_Request);
      REQUIRE_THAT(resp.err.getMessage(), Contains("unavailable"));
    }

    INFO("Starting UAVCAN client");
    auto can_client = new AgentUAVCANClient(cfg);
    a.setUavClient(can_client);

    SECTION("adcs command enabled") {
      auto cmd = AdcsCommandRequest();
      SECTION("bad command") {
        cmd.setCommand("oops");
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        INFO(cmd.getCommand());
        REQUIRE(resp.code == Code::Bad_Request);
        REQUIRE_THAT(resp.result.getReason(), StartsWith("Cannot translate"));
      }
      cmd.setCommand("IDLE");
      SECTION("bad aperture") {
        cmd.setAperture("012345678910...15...20...X");
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);
        REQUIRE_THAT(resp.result.getReason(), StartsWith("Invalid aperture"));
      }
      cmd.setAperture("GOPRO");
      SECTION("basic ok") {
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);  // unit shim always says this
        CHECK_THAT(resp.result.getReason(), StartsWith("Aperture"));
        CHECK(resp.result.getMode() == Adaptor::AcsMode::SUNSPIN);
        CHECK(!resp.result.quatIsSet());
        CHECK(!resp.result.targetIsSet());
        CHECK(!resp.result.vectorIsSet());
      }
      SECTION("quat") {
        org::openapitools::server::model::Adcs_quat_t quat;
        quat.setQ1(10);
        quat.setQ2(11);
        quat.setQ3(12);
        quat.setQ4(13);
        cmd.setQuat(quat);
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);  // unit shim always says this
        CHECK_THAT(resp.result.getReason(), StartsWith("Aperture"));
        CHECK(resp.result.getMode() == Adaptor::AcsMode::SUNSPIN);
        CHECK(resp.result.quatIsSet());
        CHECK(!resp.result.targetIsSet());
        CHECK(!resp.result.vectorIsSet());
        CHECK(resp.result.getQuat().getQ1() == 11);
        CHECK(resp.result.getQuat().getQ2() == 13);
        CHECK(resp.result.getQuat().getQ3() == 15);
        CHECK(resp.result.getQuat().getQ4() == 17);
      }
      SECTION("target") {
        org::openapitools::server::model::AdcsTarget target;
        target.setLat(3);
        target.setLon(4);
        cmd.setTarget(target);
        cmd.setAngle(13);
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);  // unit shim always says this
        CHECK(resp.result.targetIsSet());
        CHECK(!resp.result.quatIsSet());
        CHECK(!resp.result.vectorIsSet());
        CHECK(resp.result.getTarget().getLat() == 4);
        CHECK(resp.result.getTarget().getLon() == 17);
      }
      SECTION("vector") {
        org::openapitools::server::model::Adcs_xyz_float_t vector;
        vector.setX(3);
        vector.setY(2);
        vector.setZ(1);
        cmd.setVector(vector);
        auto resp = a.adcs_command(cmd);
        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);  // unit shim always says this
        CHECK(resp.result.vectorIsSet());
        CHECK(!resp.result.quatIsSet());
        CHECK(!resp.result.targetIsSet());
        CHECK(resp.result.getVector().getX() == 2);
        CHECK(resp.result.getVector().getY() == 0);
        CHECK(resp.result.getVector().getZ() == -2);
      }
      SECTION("q+t+v") {
        // all together
        org::openapitools::server::model::Adcs_quat_t quat;
        quat.setQ1(0);
        quat.setQ2(1);
        quat.setQ3(2);
        quat.setQ4(3);
        cmd.setQuat(quat);

        org::openapitools::server::model::AdcsTarget target;
        target.setLat(3);
        target.setLon(4);
        cmd.setTarget(target);
        cmd.setAngle(3);

        org::openapitools::server::model::Adcs_xyz_float_t vector;
        vector.setX(4);
        vector.setY(5);
        vector.setZ(6);
        cmd.setVector(vector);

        cmd.setCommand("NADIR");
        auto resp = a.adcs_command(cmd);
        CHECK(resp.result.getMode() == Adaptor::AcsMode::NOOP);

        INFO(resp.err.getMessage());
        INFO(resp.result.getReason());
        REQUIRE(resp.code == Code::Bad_Request);  // unit shim always says this
        CHECK(resp.result.targetIsSet());
        CHECK(resp.result.quatIsSet());
        CHECK(resp.result.vectorIsSet());

        CHECK(resp.result.getTarget().getLat() == 4);
        CHECK(resp.result.getTarget().getLon() == 7);

        CHECK(resp.result.getQuat().getQ1() == 1);
        CHECK(resp.result.getQuat().getQ2() == 3);
        CHECK(resp.result.getQuat().getQ3() == 5);
        CHECK(resp.result.getQuat().getQ4() == 7);

        CHECK(resp.result.getVector().getX() == 3);
        CHECK(resp.result.getVector().getY() == 3);
        CHECK(resp.result.getVector().getZ() == 3);
      }
    }
}
