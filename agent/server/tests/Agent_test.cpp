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

        INFO("topics");
        REQUIRE(resp.result.getTopics().empty());

        req.setTopics(vector<string>({"test"}));
        resp = a.collector_info(req);
        REQUIRE(!resp.result.getTopics().empty());


        INFO("send_file");
        char clitmpl[] = "/tmp/unittest_clientXXXXXX";
        char *ctmp = mkdtemp(clitmpl);

        // create temporary files
        char *ftmps[2];
        for (int i=0; i < 2; ++i) {
            string ftmpl(ctmp);
            ftmpl.append("/sendfileXXXXXX");

            ftmps[i] = strdup(ftmpl.c_str());
            FILE *f = fdopen(mkstemp(ftmps[i]), "wb");
            // write some data to it
            fputs("The quick brown fox jumps over the lazy dog", f);
            fclose(f);
        }

        // call send_file
        SendFileRequest sf_req;
        sf_req.setDestination("ground");
        sf_req.setFilepath(ftmps[0]);
        sf_req.setTopic("test-fail");

        auto sf_resp = a.send_file(sf_req);
        // verify that file+data+meta are in transfer directory
        REQUIRE(sf_resp.code == Code::Bad_Request);
        REQUIRE_THAT(sf_resp.err.getMessage(), StartsWith("Topic "));

        sf_req.setTopic("test");
        sf_resp = a.send_file(sf_req);
        REQUIRE(sf_resp.code == Code::Ok);

        // send a second file
        sf_req.setFilepath(ftmps[1]);
        auto sf_resp2 = a.send_file(sf_req);
        REQUIRE(sf_resp.code == Code::Ok);

        string transfer_filename(si.getWorkdir());
        transfer_filename.append("/");
        transfer_filename.append(sf_resp.result.getUUID());
        transfer_filename.append(".data.oort");

        WARN(transfer_filename);
        REQUIRE(access(transfer_filename.c_str(), F_OK) == 0);

        INFO("query_available");

        // nothing here yet
        auto qa_resp_empty = a.query_available("test");
        REQUIRE(qa_resp_empty.result.getFiles().size() == 0);

        // move "send_file" created files over to uploads directory
        rename(
             si.getWorkdir().append("/").append(sf_resp.result.getUUID().append(".data.oort")).c_str(),
             si.getUploads().append("/").append(sf_resp.result.getUUID().append(".data.oort")).c_str()
        );
        rename(
             si.getWorkdir().append("/").append(sf_resp.result.getUUID()).append(".meta.oort").c_str(),
             si.getUploads().append("/").append(sf_resp.result.getUUID()).append(".meta.oort").c_str()
        );
        // call query_available
        auto qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);

        INFO("bad meta file");
        ofstream badfile(si.getUploads().append("/abcd.meta.oort"));
        badfile << "bad file" << endl;
        badfile.close();

        // still works
        qa_resp = a.query_available("test");
        REQUIRE(qa_resp.result.getFiles().size() == 1);

        // move second "send_file" created files over to uploads directory
        rename(
             si.getWorkdir().append("/").append(sf_resp2.result.getUUID().append(".data.oort")).c_str(),
             si.getUploads().append("/").append(sf_resp2.result.getUUID().append(".data.oort")).c_str()
        );
        rename(
             si.getWorkdir().append("/").append(sf_resp2.result.getUUID()).append(".meta.oort").c_str(),
             si.getUploads().append("/").append(sf_resp2.result.getUUID()).append(".meta.oort").c_str()
        );
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
        REQUIRE(access(upload_file.c_str(), F_OK) == 0);

        // call retrieve_file transferring to temporary directory
        auto rf_resp = a.retrieve_file(rf_req);

        // verify file in new directory, deleted from uploads
        REQUIRE(access(save_path.c_str(), F_OK) == 0);
        REQUIRE(access(upload_file.c_str(), F_OK) != 0);

        // verify only 1 file remaining from query_available
        auto qa_resp_post = a.query_available("test");
        REQUIRE(qa_resp_post.result.getFiles().size() == 1);
        REQUIRE(qa_resp_post.result.overflowIsSet() == false);

        INFO("cleanup");
        // cleanup
        rmdir(si.getUploads().c_str());
        rmdir(si.getAgentUpgrades().c_str());
        rmdir(si.getWorkdir().c_str());
        rmdir(dtmp);
    }
}
