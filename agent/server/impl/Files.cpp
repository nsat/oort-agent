/**
 * Files.cpp
 *
 * Agent files utilities
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include "Files.h"

#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <system_error>  // NOLINT(build/c++11)

#include "Utils.h"

using org::openapitools::server::model::FileInfo;
using namespace std;

/**
 * \brief get the names of valid files in a directory
 * 
 * Uses file_info to filter the files in a directory and exclude non-plain files.
 */
vector<string> Files::file_names(const string &dir) {
  auto files = list_files(dir);
  vector<string> file_list;
  for (auto f = files.begin(); f != files.end(); ++f) {
    try {
      // file_stat is used as a valid item filter;   it throws runtime_error
      // on a bad file.  The return value is ignored.
      auto fs = file_stat(dir + "/" + *f);
      file_list.push_back(*f);
    } catch (const runtime_error &e) {
      // ignore errors thrown from file_stat
    }
  }
  return file_list;
}

/**
 * \brief list files in a directory
 *
 * dotfiles are skipped.  If an extension is specified,
 * only files matching that extension are returned.
 */
vector<string> Files::list_files(const string &dir, const string &ext) {
  dirent dp, *result;
  vector<string> dlist;

  DIR *dirp = opendir(dir.c_str());
  if (dirp == NULL) {
    throw system_error(errno, generic_category(), "opendir");
  }
  try {
    while (readdir_r(dirp, &dp, &result) == 0 && result != NULL) {
      string fname(dp.d_name);
      if (fname[0] != '.' &&
          (ext.empty() || ends_with(fname, ext))) {
        dlist.push_back(fname);
      }
    }
  }
  catch (...) {
    closedir(dirp);
    throw;
  }
  closedir(dirp);
  return dlist;
}

/**
 * \brief stat a file
 * 
 * Throws an error if the file is not a regular file.
 * 
 */
struct stat Files::file_stat(const std::string &file) {
    struct stat buf;

    if (stat(file.c_str(), &buf) != 0) {
        throw runtime_error("stat error: " + string(strerror(errno)));
    } else if (!S_ISREG(buf.st_mode)) {
        throw runtime_error("stat error: " + file + " is not a regular file");
    }

    return buf;
}

/**
 * \brief Build and return a FileInfo for the given file
 * 
 * Throws an error (from file_stat) if the file is not a regular file.
 * 
 * NB: will read the entire file in order to calculate the CRC.
 */
FileInfo Files::file_info(const string &file) {
    struct stat buf = Files::file_stat(file);

    FileInfo fi;
    fi.setPath(file);
    fi.setCreated(buf.st_ctime);
    fi.setModified(buf.st_mtime);
    fi.setSize(buf.st_size);
    fi.setCrc32(getCrc(file));

    fi.setId(tailname(file));
    return fi;
}

/**
 * \brief Find files in a directory older than a given age
 *
 */
vector<string> Files::oldFiles(const string &dir, const int age) {
  dirent dp, *result;
  struct stat buf;

  const int now = time(NULL);
  // TODO(jeff.rogers) check for errors
  DIR *dirp = opendir(dir.c_str());
  vector<string> dlist;

  while (readdir_r(dirp, &dp, &result) == 0 && result != NULL) {
    if (dp.d_name[0] != '.') {
      string fname(dir + "/" + dp.d_name);
      if (stat(fname.c_str(), &buf) == 0) {
        // ignore any stat errors
        if (buf.st_mtime < (now - age)) {
          dlist.push_back(fname);
        }
      }
    }
  }
  closedir(dirp);
  return dlist;
}

/**
 * \brief checks that a file path is valid
 * 
 * In the current model, all file paths must be absolute.  This is handled
 * by checking that the path starts with a '/'.
 * 
 * \param path 
 * \return true 
 * \return false 
 */
bool Files::checkPath(const std::string &path) {
    return path[0] == '/';
}
