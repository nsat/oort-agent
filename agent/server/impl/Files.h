/**
 * Files.h
 *
 * Agent files utilities
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <string>
#include <vector>

#include "FileInfo.h"

using org::openapitools::server::model::FileInfo;

namespace Files {
    std::vector<std::string> file_names(const std::string &dir);
    std::vector<std::string> list_files(const std::string &dir, const std::string &extension = "");
    FileInfo file_info(const std::string &file);
    std::vector<std::string> oldFiles(const std::string &dir, const int age);
    bool checkPath(const std::string &path);
};
