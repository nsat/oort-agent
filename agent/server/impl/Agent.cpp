/**
 * Agent.cpp
 *
 * Main agent implementation.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include "Agent.h"

#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <regex>  // NOLINT(build/c++11)
#include <system_error>  // NOLINT(build/c++11)
#include <utility>
#include <vector>

#include "Files.h"
#include "Log.h"
#include "Utils.h"

#include "version.h"  // NOLINT

using namespace std;

Agent::Agent(const AgentConfig &cfg) : cleaner(this) {
    if (!cfg.initialized) {
        throw runtime_error("configuration not initialized");
    }
    struct utsname buf;
    if (uname(&buf) != 0) {
        throw system_error(errno, generic_category(), "initializing uname");
    }
    nodename = buf.nodename;
    machine = buf.machine;

    workdir = cfg.workdir;
    transfer_dir = workdir + "/transfers";
    upload_dir = workdir + "/uploads";
    upgrade_dir = workdir + "/upgrades";
    deadletter_dir = workdir + "/dead";
    vector<string> all_dirs = {transfer_dir, upload_dir, upgrade_dir, deadletter_dir};
    create_dirs(all_dirs);

    cleaner.setCleanupDirs(all_dirs);
    cleaner.setCleanupInterval(cfg.cleanupInterval);
    cleaner.setMaxAge(cfg.maxAge);
    cleaner.scheduleCleanup();

    uavcan_client = nullptr;
}

Agent::~Agent() {
    Log::warn("stopping cleanup worker");
    cleaner.stopWorker();
}

Cleaner& Agent::getCleaner() {
    return cleaner;
}

void Agent::setUavClient(AgentUAVCANClient *client) {
    uavcan_client = client;
}

void Agent::create_dirs(const std::vector<string> &dirs) {
    for (string dir : dirs) {
        if (mkdir(dir.c_str(), 0700) != 0) {
            if (errno != EEXIST) {
                Log::error("Error initializing directory ?: ?",
                          dir, OSError(""));
                throw runtime_error("Error initializing directory " + dir);
            }
        }
    }
}

ResponseCode<AvailableFilesResponse> Agent::query_available(
    const string &topic
    ) {
    ResponseCode<AvailableFilesResponse> resp;
    auto files = files_info(upload_dir, topic);
    if (files.size() > max_query) {
        // NB: files_info returns a list of size max_query+1 on overflow.
        resp.result.setOverflow(true);
        files.pop_back();
    }
    resp.result.setFiles(files);
    resp.code = Code::Ok;
    return resp;
}

/**
 * \brief read .meta files and filter based on topic
 * 
 * The size of the returned file list is limited to `max_query + 1`.  Callers
 * should check the size of the returned list and, if it is too long,
 * flag an overflow and delete the final element, leaving the list at the
 * correct maximum length.
 */
vector<FileInfo> Agent::files_info(const string &dir, const string &topic) {
  auto meta_files = Files::list_files(dir, META_EXT);
  vector<FileInfo> flist;
  for (auto it = meta_files.begin(); it != meta_files.end(); ++it) {
    try {
      auto tm = read_transfer_meta(dir + "/" + *it);
      // check topic
      if (tm.getTopic() != topic) {
          continue;
      }
      auto fi = Files::file_info(data_file(dir + "/" + *it));
      // verify crc between stored and calculated
      if (tm.getFileInfo().getCrc32() != fi.getCrc32()) {
          Log::warn("CRC mismatch on ?; endeadening",  chop(*it, META_EXT));
          endeaden(dir + "/" + *it);
          continue;
      }
      flist.push_back(tm.getFileInfo());
    } catch (const runtime_error &e) {
      // errors from file_info could be a missing file, or a non-file file
      // error reading the transfer meta could be a corrupt or non-schema
      // file
      Log::error("Error in files_info handling ?: ?", *it, e.what());
      endeaden(dir + "/" + *it);
    }
    if (flist.size() > max_query) {
        break;
    }
  }
  return flist;
}

TransferMeta Agent::transfer_meta(const SendFileRequest &req, const FileInfo &fi) {
    TransferMeta tm;
    tm.setMetaVersion(getApiVersion());
    tm.setTopic(req.getTopic());
    tm.setDestination(req.getDestination());
    tm.setNode(nodename);
    tm.setTime(time(NULL));
    tm.setFileInfo(fi);
    tm.setSendOptions(req.getOptions());
    return tm;
}

TransferMeta Agent::read_transfer_meta(const string &file) {
    TransferMeta tm;
    ifstream metafile(file);

    if (metafile.fail()) {
        Log::error("Error opening ?: ? - endeadening", file, OSError());
        endeaden(file);
        // throw a runtime error
        // Can't rely on ifstream::failure, see
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
        throw runtime_error("error reading metadata");
    }
    try {
        from_jsonfile(metafile, tm);
    }
    catch (const nlohmann::json::exception &e) {
        // log parse errors, then rethrow as a runtime error
        Log::error("Error parsing ?: ? - endeadening", file, e.what());
        endeaden(file);
        throw runtime_error("error parsing metadata");
    }

    return tm;
}

ResponseCode<PingResponse> Agent::ping() {
    ResponseCode<PingResponse> resp;

    resp.code = Code::Ok;
    resp.result = PingResponse();
    resp.result.setResponse("PONG");
    return resp;
}

ResponseCode<TransferMeta> Agent::meta(const std::string &uuid) {
    ResponseCode<TransferMeta> resp;

    try {
        resp.result = read_transfer_meta(transfer_dir + "/" + uuid + META_EXT);
        resp.code = Code::Ok;
        return resp;
    }
    catch (const runtime_error &e) {
        resp.err.setMessage(e.what());
        resp.code = Code::Bad_Request;
        return resp;
    }
}

ResponseCode<SendFileResponse> Agent::send_file(const SendFileRequest &req) {
    string src = req.getFilepath();
    string id = mkUUID();
    string dest = transfer_dir + "/" + id + DATA_EXT;
    string destmeta = transfer_dir + "/" + id + META_EXT;
    FileInfo fi;
    string topic = req.getTopic();
    ResponseCode<SendFileResponse> resp;

    if (!Files::checkPath(src)) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("File path " + src + " is not absolute");
        return resp;
    }
    if (!checkTopic(topic)) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Topic " + topic + " is not allowed");
        return resp;
    }

    // initial check: verify that the source directory is writable,
    // so that the file can be deleted at the end
    if (access(dirname(src).c_str(), W_OK) != 0) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Source directory not writable");
        return resp;
    }

    try {
        fi = Files::file_info(src);
        fi.setId(id);
        TransferMeta tm = transfer_meta(req, fi);

        ofstream meta(destmeta);
        meta << to_jsonstr(tm);
        if (meta.fail()) {
            Log::error("Error writing metadata file ?: ?", destmeta, OSError());
            unlink(destmeta.c_str());
            throw runtime_error("error writing metadata");
        }
    } catch (const runtime_error &e) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage(OSError("Error getting info on " + src + ": "));
        return resp;
    }

    try {
        move_file(src, dest, fi);
    } catch (const system_error &e) {
        // remove meta file on error
        unlink(destmeta.c_str());
        Log::error("error in send_file: ?", e.what());
        resp.code = Code::Bad_Request;
        resp.err.setMessage(e.what());
        return resp;
    }

    resp.code = Code::Ok;
    resp.result.setUUID(id);
    return resp;
}

ResponseCode<FileInfo> Agent::retrieve_file(
    const RetrieveFileRequest &req
    ) {
    ResponseCode<FileInfo> resp;
    string srcid = req.getId();
    string dest = req.getSavePath();

    string srcfile = upload_dir + "/" + srcid + DATA_EXT;
    string src_meta = upload_dir + "/" + srcid + META_EXT;

    if (!Files::checkPath(dest)) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Destination path " + dest + " is not absolute");
        return resp;
    }
    if (access(src_meta.c_str(), R_OK) != 0) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage(
            OSError("meta file unreadable for " + srcid + ": "));
        return resp;
    }

    try {
        auto tm = read_transfer_meta(src_meta);
        move_file(srcfile, dest, tm.getFileInfo());
        resp.result = FileInfo(tm.getFileInfo());
    } catch (const system_error &err) {
        // don't endeaden the file in this case, since the error
        // was related to the state of the system and NOT the file itself
        Log::error("system error in retrieve_file: ?", err.what());
        resp.code = Code::Bad_Request;
        resp.err.setMessage(err.what());
        return resp;
    } catch (const runtime_error &err) {
        endeaden(srcfile);
        Log::error("error in retrieve_file, endeadening ?: ?", srcid, err.what());
        resp.code = Code::Bad_Request;
        resp.err.setMessage(err.what());
        return resp;
    }
    // TODO(jeff.rogers): check for errors
    unlink(src_meta.c_str());

    resp.code = Code::Ok;
    return resp;
}

ResponseCode<AdcsResponse> Agent::adcs_get() {
    ResponseCode<AdcsResponse> resp;

    if (adcs == nullptr) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Adcs interface is unavailable");
        return resp;
    }
    resp.code = Code::Ok;
    resp.result = adcs->getAdcs();
    return resp;
}

ResponseCode<AdcsSetResponse> Agent::adcs_set(const AdcsSetRequest &req) {
    ResponseCode<AdcsSetResponse> resp;

    if (uavcan_client == nullptr) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Adcs interface is unavailable");
        return resp;
    }
    resp.code = Code::Ok;
    uavcan_client->AdcsSet(req, resp.result);
    return resp;
}

ResponseCode<TfrsResponse> Agent::tfrs_get() {
    ResponseCode<TfrsResponse> resp;

    if (adcs == nullptr) {
        resp.code = Code::Bad_Request;
        resp.err.setMessage("Tfrs interface is unavailable");
        return resp;
    }
    resp.code = Code::Ok;
    resp.result = adcs->getTfrs();
    return resp;
}

int64_t Agent::getDiskfree() {
    struct statvfs vfs;

    statvfs(workdir.c_str(), &vfs);
    return diskFree(vfs);
}

int64_t Agent::diskFree(const struct statvfs &vfs) {
    return vfs.f_frsize * vfs.f_bavail;
}

int64_t Agent::diskSize(const struct statvfs &vfs) {
    return vfs.f_frsize * vfs.f_blocks;
}

int Agent::diskFreePct(const struct statvfs &vfs) {
    int64_t fs_total = vfs.f_frsize * vfs.f_blocks;
    return (diskFree(vfs) * 100) / fs_total;
}

bool Agent::checkDiskFree(int64_t sz) {
    struct statvfs vfs;
    if (min_free_pct == -1 && min_free_pct == -1) {
        // if no limits have been set, just return true and skip the statvfs
        return true;
    }
    statvfs(workdir.c_str(), &vfs);
    return checkDiskFree(sz, vfs);
}

bool Agent::checkDiskFree(int64_t sz, const struct statvfs &vfs) {
    int64_t free_needed = sz;
    if (min_free_disk > 0) {
        free_needed = sz + (min_free_disk * 1024*1024);
    }
    if (min_free_pct > 0) {
        free_needed = max(free_needed, sz + (min_free_pct * diskSize(vfs) / 100));
    }
    Log::info("Free space required: ?  available: ?", free_needed, diskFree(vfs));
    return (diskFree(vfs) >= free_needed);
}

void Agent::setMinFreeMb(int mb) {
    Log::info("setting minimum free disk ? megabytes", mb);
    min_free_disk = mb;
}

void Agent::setMinFreePct(int pct) {
    Log::info("setting minimum free disk ? percent", pct);
    min_free_pct = pct;
}

void Agent::setMaxQuery(int max) {
    Log::info("setting max query size to ?", max);
    max_query = max;
}

string Agent::getUsername() {
    size_t buflen = 128;  // initial size; generally enough
    const size_t max_buflen = 10 * 1024;  // generous maximum size for struct passwd strings
    Log::debug("getUsername buflen = ?", static_cast<int>(buflen));
    vector<char> buffer;
    struct passwd pwd, *pwdr;
    uid_t uid = getuid();
    while (buflen < max_buflen) {
        buffer.reserve(buflen);
        int res = getpwuid_r(uid, &pwd, buffer.data(), buflen, &pwdr);
        if (res != ERANGE) {
            break;
        }
        buflen *= 2;
        Log::debug("getUsername: increasing buffer length to ?", static_cast<int>(buflen));
    }
    if (pwdr == NULL) {
        Log::error("getUsername: user not found for uid ?", static_cast<int>(uid));
        return {};
    }
    return pwd.pw_name;
}

SystemInfo Agent::getSysinfo() {
    SystemInfo si;

    si.setVersion(getReleaseVersion());
    si.setBuild(getBuildVersion());
    si.setDiskfree(getDiskfree());
    si.setWorkdir(transfer_dir);
    si.setUploads(upload_dir);
    si.setAgentUpgrades(upgrade_dir);
    si.setDead(deadletter_dir);
    si.setUsername(getUsername());
    si.setNodename(nodename);
    si.setMachine(machine);

    return si;
}

ResponseCode<InfoResponse> Agent::collector_info(InfoRequest &req) {
    ResponseCode<InfoResponse> resp;

    if (req.topicsIsSet()) {
        m_allowedTopics = move(req.getTopics());
    }

    resp.result.setSysinfo(getSysinfo());
    resp.result.setTopics(m_allowedTopics);

    InfoResponse_available available;
    available.setFiles(Files::file_names(transfer_dir));
    available.setDead(Files::file_names(deadletter_dir));
    resp.result.setAvailable(available);

    resp.code = Code::Ok;
    return resp;
}


bool Agent::checkTopic(const std::string &topic) {
    if (!regex_match(topic, topic_re)) {
        // validate against topic regexp
        return false;
    } else if (m_allowedTopics.empty()) {
        // no topics set means anything is allowed
        return true;
    } else {
        // linear search of allowed topics.  Assumption is that the list of
        // allowed topics will be small (less than 10), so it's
        // not worth using a more complex strategy.
        if (find(m_allowedTopics.begin(), m_allowedTopics.end(), topic) != m_allowedTopics.end()) {
            return true;
        } else {
            return false;
        }
    }
}

/*
 * move a file and its pair to the dead-letter box
 * 
 * endeaden - verb; make something dead or more dead.  Opposite of 'enliven'.
 * 
 * When file corruption is detected, the file should be endeadened, so that it
 * does not *repeatedly* get detected, and logged, as corrupt.  Since the
 * files come in pairs (.data and .meta) the other side of the pair is moved as well.
 * The dead-letter folder should be on the same filesystem as the other
 * working directories, so a simple atomic rename should work in all cases.
 */
void Agent::endeaden(const string &filepath) {
    [[unlikely]] if (!(starts_with(filepath, transfer_dir) ||
          starts_with(filepath, upload_dir) ||
          starts_with(filepath, upgrade_dir))) {
        Log::error("attempt to endeaden a file outside working directory: ?", filepath);
        return;
    }

    string twin;
    if (ends_with(filepath, META_EXT)) {
        twin = data_file(filepath);
    } else if (ends_with(filepath, DATA_EXT)) {
        twin = meta_file(filepath);
    } else {
        Log::error("attempt to endeaden a non-oort file: ?", filepath);
        return;
    }

    string new_path = deadletter_dir + "/" + tailname(filepath);
    if (rename(filepath.c_str(), new_path.c_str()) != 0) {
        Log::error("attempt to endeaden ? failed: ?", filepath, OSError(""));
    }
    string new_twin_path = deadletter_dir + "/" + tailname(twin);
    if (rename(twin.c_str(), new_twin_path.c_str()) != 0) {
        Log::error("attempt to endeaden (twin) ? failed: ?", twin, OSError(""));
    }
}

/**
 * \brief move a file
 * 
 * Move a file from src to dest.   If the src and dest are on the same filesystem,
 * then this is a simple rename.  If they aren't, then the file has to be copied
 * over, and the original removed.   As there are various things that can fail
 * for that copy, this method does a lot of checking, and if anything goes wrong
 * it sets things back the way they were before.
 * 
 * XDEV handling steps:
 * 1. Check for available disk space on the destination.
 * 2. Create a temporary directory in the source dir, and rename the
 *    source file into that directory.  The rename here is an atomic 
 *    operation here, and gives us a guarantee that the source file 
 *    can be deleted afterwards.  
 * 
 * After the source file is renamed, any error should result in the 
 * rename being reverted.
 * 
 * 3. Create a temporary file in the destination directory.  This
 *    gives us a guarantee that the tempfile can be atomically renamed
 *    to the destination file.
 * 4. Open the renamed source file for reading.
 * 5. Read a block at a time from the renamed source file and write 
 *    to the temporary dest file.  The block size used is the smaller of
 *    the block sizes from the source and destination filesystems.
 * 6. Verify the CRC of the temporary destination file against the original.
 * 7. Rename the temporary destination file to the final destination file.
 * 8. Remove the source temporary file and directory.
 * 
 * If any error happened in steps 3-8:
 * E1. rename the source temporary file back to the original filename
 * E2. remove the source temporary directory
 * E3. propagate the error
 */
void Agent::move_file(const string &src, const string &dest, const FileInfo &fi) {
    // abort if destination file exists
    if (access(dest.c_str(), F_OK) == 0) {
        throw system_error(EEXIST, generic_category(), "move_file error - file exists");
    }
    FileInfo precheck = Files::file_info(src);
    if (precheck.getCrc32() != fi.getCrc32()) {
        throw runtime_error("move_file: CRC Check failed on source file");
    }
    if (rename(src.c_str(), dest.c_str()) == 0) {
        // easy path: same filesystem
        return;
    } else if (errno == EXDEV) {
        // cross-device move.   Copy, with intermediate renaming.

        struct statvfs srcvfs, destvfs;
        // get vfs info, for free space and block sizing guidance
        statvfs(dirname(src).c_str(), &srcvfs);
        statvfs(dirname(dest).c_str(), &destvfs);

        if (!checkDiskFree(fi.getSize(), destvfs)) {
            throw system_error(EDQUOT, generic_category(),
                "insufficient disk space for destination file");
        }

        string src_tmpdir = dirname(src) + "/file_move_XXXXXX";
        vector<char> s_tmpdirname(src_tmpdir.begin(), src_tmpdir.end());
        s_tmpdirname.push_back('\0');
        char *res = mkdtemp(s_tmpdirname.data());
        if (res[0] == '\0') {
            throw system_error(errno, generic_category(), "mkdtemp error (xdev)");
        }
        string s_tmpfilename(s_tmpdirname.data());
        s_tmpfilename.append("/file_move");
        Log::debug("src_tmp ?", s_tmpfilename);
        if (rename(src.c_str(), s_tmpfilename.c_str()) != 0) {
            throw system_error(errno, generic_category(), "rename error (xdev)");
        }
        try {
            // create temporary file in dest directory
            string dest_tmp = dirname(dest) + "/file_move_XXXXXX";
            vector<char> dtname(dest_tmp.begin(), dest_tmp.end());
            dtname.push_back('\0');

            int dest_fd = mkstemp(dtname.data());
            if (dest_fd == -1) {
                throw system_error(errno, generic_category(), "mkstemp error (xdev)");
            }
            int src_fd = open(s_tmpfilename.c_str(), O_RDONLY);
            if (src_fd == -1) {
                throw system_error(errno, generic_category(), "open error (xdev)");
            }

            int blksize = min(srcvfs.f_bsize, destvfs.f_bsize);
            vector<char> buffer(blksize);
            int rcount, wcount;
            while ((rcount = read(src_fd, buffer.data(), blksize)) > 0) {
                Log::debug("copy ? bytes", rcount);
                wcount = write(dest_fd, buffer.data(), rcount);
                if (wcount != rcount) {
                    Log::error("written bytes differ from read: ? != ?", wcount, rcount);
                }
            }
            close(src_fd);
            fsync(dest_fd);
            close(dest_fd);

            FileInfo dest_fi = Files::file_info(dtname.data());
            if (dest_fi.getCrc32() != fi.getCrc32()) {
                throw runtime_error("move_file: CRC Check failed on written file");
            }
            rename(dtname.data(), dest.c_str());
            unlink(s_tmpfilename.c_str());
            rmdir(s_tmpdirname.data());
        } catch (...) {
            // if anything went wrong after src has been renamed,
            // undo the rename to leave src in place
            rename(s_tmpfilename.c_str(), src.c_str());
            rmdir(s_tmpdirname.data());
            throw;
        }
    } else {
        throw system_error(errno, generic_category(), "Error renaming");
    }
}

string Agent::getBuildVersion() {
    return BUILD_VERSION;
}

string Agent::getReleaseVersion() {
    return RELEASE_VERSION;
}

string Agent::getApiVersion() {
    return API_VERSION;
}
