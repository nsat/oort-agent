
#include <unistd.h>
#include <unordered_set>

#include "catch2/catch.hpp"

#include "Files.h"

using namespace std;

TEST_CASE( "file_info checks", "[file]") {
    char worktmpl[] = "/tmp/unittest_filesXXXXXX";
    string dtmp = string(mkdtemp(worktmpl));

    INFO("regular file") {
        string fname = dtmp + "/thefile";
        string cmd = "touch " + fname;
        system(cmd.c_str());
        REQUIRE_NOTHROW( Files::file_info(fname) );
        REQUIRE_NOTHROW( Files::file_stat(fname) );

        auto fi = Files::file_info(fname);
        REQUIRE( fi.getSize() == 0);
        REQUIRE( fi.getId() == "thefile");
    }

    INFO("detect named pipe") {
        // create a named pipe in the temp dir
        string fname = dtmp + "/thepipe";
        string cmd = "mknod " + fname + " p";
        system(cmd.c_str());

        REQUIRE_THROWS( Files::file_info(fname) );
        REQUIRE_THROWS( Files::file_stat(fname) );
    }

    INFO("detect directory") {
        // create a subdirectory in the temp dir
        string fname = dtmp + "/thedir";
        string cmd = "mkdir " + fname;
        system(cmd.c_str());

        REQUIRE_THROWS( Files::file_info(fname) );
        REQUIRE_THROWS( Files::file_stat(fname) );
    }

    INFO("no file") {
        // file not found
        string fname = dtmp + "/nofile";

        REQUIRE_THROWS( Files::file_info(fname) );
        REQUIRE_THROWS( Files::file_stat(fname) );
    }

    INFO("list whats left") {
        auto all = Files::list_files(dtmp);
        // convert to unordered set so directory order doesn't matter
        unordered_set<string> expected_all {"thefile", "thepipe", "thedir"};
        REQUIRE( unordered_set<string>(all.begin(), all.end()) == expected_all);

        vector<string> expected_valid {"thefile"};
        REQUIRE( Files::file_names(dtmp) == expected_valid);
    }

}
