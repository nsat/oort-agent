/*
 * Utils.cpp
 * 
 * Utility functions.
 * 
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include <zlib.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>  // NOLINT(build/c++11)
#include <vector>

#include "Log.h"
#include "Utils.h"

using namespace std;

string mkUUID() {
    char rand_dat[16];

    ifstream urand("/dev/urandom");
    urand.read(rand_dat, sizeof(rand_dat));
    urand.close();

    rand_dat[6] = 0x40 | (rand_dat[6] & 0x0F);
    rand_dat[8] = 0x80 | (rand_dat[8] & 0x3F);

    stringstream uuid;
    uuid << hex << setfill('0');
    int i = 0;
    // time-low
    for (int c = 0; c < 4; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // time-mid
    for (int c = 0; c < 2; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // time-high-and-version
    for (int c = 0; c < 2; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // clock-seq-and-reserved
    for (int c = 0; c < 1; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    // clock-seq-low
    for (int c = 0; c < 1; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    uuid << '-';
    // node
    for (int c = 0; c < 6; c++, i++) uuid << setw(2) << (0xFF & rand_dat[i]);
    return uuid.str();
}

string OSError(const string &prefix) {
    stringstream errmsg;
    errmsg << prefix;
    char buf[100];
    // Annoyingly, gnu libc uses the same function prototype for strerror_r,
    // but completely different behavior from POSIX.
    // strerror_r(errno, buf, 100);
    // errmsg << buf;
    errmsg << strerror_r(errno, buf, sizeof(buf));
    return errmsg.str();
}

string getCrc(const string &file) {
    constexpr int kBlocksize = 1024;
    char c[kBlocksize];
    ulong crc = crc32(0L, 0, 0);
    ifstream ifs(file);
    do {
        ifs.read(c, kBlocksize);
        crc = crc32(crc, (unsigned char*)c, ifs.gcount());
    } while (ifs.rdstate() == ios::goodbit);

    stringstream crcx;
    crcx << hex << setfill('0') << setw(8) << crc;
    return crcx.str();
}

/**
 * return the portion of the file path after the last "/"
 */
string tailname(const string &filepath) {
    auto basedir = filepath.rfind("/");
    if (basedir == string::npos) {
        return filepath;
    } else {
        return filepath.substr(basedir + 1);
    }
}

/**
 * return the portion of the file path before the last "/"
 */
string dirname(const string &filepath) {
    auto basedir = filepath.rfind("/");
    if (basedir == string::npos) {
        return filepath;
    } else {
        return filepath.substr(0, basedir);
    }
}

/**
 * return the portion of the file name before the last "."
 * 
 * Note that this is strictly textual, so the root of 
 * "foo.tar.gz" is "foo.tar" not "foo"
 */
string rootname(const string &filepath) {
    auto ext = filepath.rfind(".");
    if (ext == string::npos) {
        return filepath;
    } else {
        return filepath.substr(0, ext);
    }
}

/**
 * return the portion of the file name after the last ".", including the "."
 * If no extension, return empty string.
 */
string extension(const string &filepath) {
    auto ext = filepath.rfind(".");
    if (ext == string::npos) {
        return "";
    } else {
        return filepath.substr(ext);
    }
}

/**
 * checks if the given string ends with the tail. 
 * 
 */
bool ends_with(const string &str, const string &tail) {
    return (str.length() >= tail.length()) &&
        (str.substr(str.length() - tail.length()) == tail);
}

/**
 * checks if the given string starts with the head. 
 * 
 */
bool starts_with(const string &str, const string &head) {
    return (str.length() >= head.length()) &&
        (str.substr(0, head.length()) == head);
}


/**
 * removes a given tail from a string
 * If the string does not end with the tail, returns
 * the original string.
 * 
 */
string chop(const string &str, const string &tail) {
    auto s_tail = str.substr(str.length() - tail.length());
    if (s_tail != tail) {
        return str;
    } else {
        return str.substr(0, str.length() - tail.length());
    }
}

/**
 * removes a given head from a string
 * If the string does not start with the head, returns
 * the original string.
 * 
 */
string behead(const string &str, const string &head) {
    auto s_head = str.substr(0, head.length());
    if (s_head != head) {
        return str;
    } else {
        return str.substr(head.length());
    }
}
