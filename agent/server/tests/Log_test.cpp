#include "catch2/catch.hpp"

#include "Log.h"
#include <iostream>
#include <sstream>

using namespace std;

// skip 23 chars for timestamp
#define TIMESTAMP_LEN 23

TEST_CASE( "Logging levels", "[log]" ) {
    stringstream dummy_out;
    string line;
    Log::setLevel(Log::Info);
    Log::setOut(dummy_out);
    SECTION( "info" ) {
        Log::info("Hello, world");

        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "info [main:0] Hello, world" );
    }
    SECTION( "debug (info level)" ) {
        Log::debug("Debug, world");

        getline(dummy_out, line);
        REQUIRE( line.empty() );
    }
    SECTION( "warn (info level)" ) {
        Log::warn("Warn, world");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warn, world" );
    }

    SECTION( "error (info level)" ) {
        Log::error("Error, world");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, world" );
    }

    Log::setLevel(Log::Warn);

    SECTION( "various (warn level)" ) {
        Log::debug("Debug, world");
        Log::info("Info, world");
        Log::warn("Warning, world");
        Log::error("Error, world");

        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warning, world" );
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, world" );
    }

    Log::setLevel(Log::Error);

    SECTION( "various (error level)" ) {
        Log::debug("Debug, world");
        Log::info("Info, world");
        Log::warn("Warning, world");
        Log::error("Error, world");

        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, world" );
    }
}

TEST_CASE( "Logging with params", "[log]" ) {
    stringstream dummy_out;
    string line;
    Log::setLevel(Log::Info);
    Log::setOut(dummy_out);
    Log::setLevel(Log::Warn);

    SECTION( "various (warn level)" ) {
        Log::debug("Debug, ? world", "one");
        Log::info("Info, ? world", "two");
        Log::warn("Warning, ? world", "three");
        Log::error("Error, ? world", "four");

        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warning, three world" );
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, four world" );
    }
    SECTION( "two params (warn level)" ) {
        Log::warn("Warning, ? ?", "world", "three");
        Log::error("Error, ? world ?", "four", "nine");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warning, world three" );
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, four world nine" );
    }
    SECTION( "two params, int conversion (warn level)" ) {
        Log::warn("Warning, ? ?", "world", 3);
        Log::error("Error, ? world ?", 4, "nine");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warning, world 3" );
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, 4 world nine" );
    }
    SECTION( "default params (warn level)" ) {
        Log::warn("Warning, world ??", "three");
        Log::error("Error, ? world ?", "four");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warning, world three<>" );
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "error [main:0] Error, four world <>" );
    }
}

TEST_CASE( "changing log levels", "[log]" ) {
    stringstream dummy_out;
    string line;
    Log::setOut(dummy_out);
    SECTION( "Start at info level level" ) {
        Log::setLevel(Log::Info);

        Log::debug("Debug, world");
        getline(dummy_out, line);
        REQUIRE( line.empty() );
        dummy_out.clear();
        // NB: dummy_out.clear() must be called after any `empty()` assertion
        // to reset the eof bit.

        SECTION( "level set to debug" ) {
            Log::setLevel(Log::Debug);
            Log::debug("Debug, world");
            getline(dummy_out, line);
            REQUIRE( line.substr(TIMESTAMP_LEN) == "debug [main:0] Debug, world" );

            SECTION( "level set back to info") {
                Log::setLevel(Log::Info);
                Log::debug("Debug, sun");
                getline(dummy_out, line);
                REQUIRE( line.empty() );
                dummy_out.clear();
            }
        }

        Log::warn("Warn, moon");
        getline(dummy_out, line);
        REQUIRE( line.substr(TIMESTAMP_LEN) == "warn [main:0] Warn, moon" );
    }
}
