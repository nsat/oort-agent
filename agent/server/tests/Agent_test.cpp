#include "catch2/catch.hpp"

#include <unistd.h>
#include <fstream>
#include "Config.h"
#include "Agent.h"
#include "Log.h"

using namespace std;
using Catch::Matchers::StartsWith;

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
