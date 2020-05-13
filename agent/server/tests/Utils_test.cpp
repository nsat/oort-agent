
#include <unistd.h>

#include "catch2/catch.hpp"

#include "Utils.h"

using namespace std;

TEST_CASE( "Length of generated UUID", "[uuid]") {
    REQUIRE( mkUUID().size() == 36 );
}
// weak test, would only catch if it was caching the same
// value forever
TEST_CASE( "UUID uniqueness", "[uuid]") {
    REQUIRE( mkUUID() != mkUUID() );
}

/**
 * path utility tests
 */
TEST_CASE( "tailname", "[paths]" ) {
    SECTION( "1.0 - 1-level path" ) {
        REQUIRE( tailname("foo/bar") == "bar" );
    }
    SECTION( "1.1 - 2-level path" ) {
        REQUIRE( tailname("foo/bar/bar") == "bar" );
    }
    SECTION( "tailname 1.2 - long path" ) {
        REQUIRE( 
            tailname("a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z") == "z");
    }
    SECTION( "tailname 1.3 - no separator" ) {
        REQUIRE( tailname("foo") == "foo" );
    }
}
TEST_CASE( "dirname", "[paths]" ) {
    SECTION( "1.0 - 1-level path" ) {
        REQUIRE( dirname("foo/bar") == "foo" );
    }
    SECTION( "1.1 - 2-level path" ) {
        REQUIRE( dirname("foo/bar/bar") == "foo/bar" );
    }
    SECTION( "dirname 1.2 - long path" ) {
        REQUIRE( 
            dirname("a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z") ==
              "a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y");
    }
    SECTION( "dirname 1.3 - no separator" ) {
        REQUIRE( dirname("foo") == "foo" );
    }
}
TEST_CASE( "rootname", "[paths]") {
    SECTION( "1.0 - normal filename") {
        REQUIRE( rootname("foo.bar") == "foo" );
    }
    SECTION( "1.1 - multiple dots") {
        REQUIRE( rootname("wee.woo.foo.bar") == "wee.woo.foo" );
    }
    SECTION( "1.2 - no dots") {
        REQUIRE( rootname("bar") == "bar" );
    }
}
TEST_CASE( "extension", "[paths]") {
    SECTION( "1.0 - normal filename") {
        REQUIRE( extension("foo.bar") == ".bar" );
    }
    SECTION( "1.1 - multiple dots") {
        REQUIRE( extension("wee.woo.foo.bar") == ".bar" );
    }
    SECTION( "1.2 - no dots") {
        REQUIRE( extension("bar") == "" );
    }
}
TEST_CASE( "root/tail.extension combined", "[paths]") {
    string fname("root/tail.extension");
    SECTION( "1.0 - plain file") {
        CHECK( tailname(fname) == "tail.extension" );
        CHECK( rootname(fname) == "root/tail" );
        CHECK( extension(fname) == ".extension" );
        CHECK( rootname(tailname(fname)) == "tail" );
        CHECK( tailname(rootname(fname)) == "tail" );
    }
}
TEST_CASE("ends_with", "[paths]") {
    string base("foo.bar.baz");
    SECTION("1.0 - base") {
        CHECK(ends_with(base, ".baz"));
        CHECK(ends_with(base, ".bar.baz"));
        CHECK(!ends_with(base, ".bar"));
        CHECK(ends_with(base, ""));
        CHECK(!ends_with(base, "a.longer.string"));
    }
}
TEST_CASE("chop", "[paths]") {
    string base("foo.bar.baz");
    SECTION("1.0 - base") {
        CHECK(chop(base, ".baz") == "foo.bar");
        CHECK(chop(base, ".bar.baz") == "foo");
        CHECK(chop(base, ".bar") == "foo.bar.baz");
        CHECK(chop(base, "") == "foo.bar.baz");
    }
}

// fragile, because the messages could change
TEST_CASE( "OSError - fragile test!", "[log]") {
    GIVEN( "errno = EACCES" ) {
        errno = EACCES;
        
        REQUIRE( OSError("test:") == "test:Permission denied" );
    }
}

// sample string/crc from https://rosettacode.org/wiki/CRC-32
TEST_CASE( "check crc" ) {
    char ftmp[] = "/tmp/unittest_crcXXXXXX";
    FILE *f = fdopen(mkstemp(ftmp), "wb");
    fputs("The quick brown fox jumps over the lazy dog", f);
    fclose(f);

    CHECK( getCrc(ftmp) == "414fa339" );

    unlink(ftmp);
}

// other samples, especially with leading 0s
TEST_CASE( "check crc 'Check 77'" ) {
    char ftmp[] = "/tmp/unittest_crcXXXXXX";
    FILE *f = fdopen(mkstemp(ftmp), "wb");
    fputs("Check 77", f);
    fclose(f);

    CHECK( getCrc(ftmp) == "0012b848" );

    unlink(ftmp);
}

TEST_CASE( "check crc 'Check_42'" ) {
    char ftmp[] = "/tmp/unittest_crcXXXXXX";
    FILE *f = fdopen(mkstemp(ftmp), "wb");
    fputs("Check_42", f);
    fclose(f);

    CHECK( getCrc(ftmp) == "04f83069" );

    unlink(ftmp);
}
