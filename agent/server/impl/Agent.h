/**
 * Agent.h
 *
 * Main agent implementation header.
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#pragma once

// for 64-bit vfs fields
#define _FILE_OFFSET_BITS 64

#include <string>
#include <vector>
#include <regex>  // NOLINT(build/c++11)

#include "AgentUAVCANClient.h"
#include "Cache.h"
#include "Cleaner.h"
#include "Config.h"
#include "Adcs.h"
#include "AdcsResponse.h"
#include "AdcsCommandRequest.h"
#include "AdcsCommandResponse.h"
#include "AvailableFilesResponse.h"
#include "Files.h"
#include "FileInfo.h"
#include "InfoRequest.h"
#include "InfoResponse.h"
#include "PingResponse.h"
#include "RetrieveFileRequest.h"
#include "SendFileRequest.h"
#include "SendFileResponse.h"
#include "SystemInfo.h"
#include "TfrsResponse.h"
#include "TransferMeta.h"
#include "Utils.h"

class Agent {
    Cleaner cleaner;
    friend class Cleaner;

    AgentUAVCANClient *uavcan_client;
    AdcsManager *adcs = nullptr;

    const std::string META_EXT = ".meta.oort";
    const std::string DATA_EXT = ".data.oort";
    std::string meta_file(const std::string &datafile) const {
        return chop(datafile, DATA_EXT) + META_EXT;
    }
    std::string data_file(const std::string &metafile) const {
        return chop(metafile, META_EXT) + DATA_EXT;
    }
    const std::regex topic_re = std::regex("^[-_[:alnum:]]+$", std::regex_constants::extended);
    // file caches
    Cache<std::vector<std::string>> dir_cache{"Directory", Files::file_names};
    // get function is Files::list_files
    Cache<std::vector<std::string>> metafile_cache{"Metafile"};
    // get function is Agent::read_transfer_meta
    Cache<TransferMeta> meta_cache{"Metainfo", 10000};

    // config values
    std::string workdir;
    int max_query = 50;

    std::vector<std::string> m_allowedTopics;

    int min_free_disk = -1;
    int min_free_pct = -1;

    std::string transfer_dir;
    std::string upload_dir;
    std::string upgrade_dir;
    std::string deadletter_dir;

    std::string nodename;
    std::string machine;

    void create_dirs(const std::vector<std::string> &dirs);

    std::vector<FileInfo> files_info(const std::string &dir, const std::string &topic);
    TransferMeta read_transfer_meta_cached(const std::string &file);
    TransferMeta read_transfer_meta(const std::string &file);
    TransferMeta transfer_meta(const SendFileRequest &req, const FileInfo &fi);

    bool checkTopic(const std::string &topic);

    // collector stuff
    std::string getUsername();
    int64_t getDiskfree();
    int64_t diskFree(const struct statvfs &vfs);
    int64_t diskSize(const struct statvfs &vfs);
    int diskFreePct(const struct statvfs &vfs);
    bool checkDiskFree(int64_t sz);
    bool checkDiskFree(int64_t sz, const struct statvfs &vfs);

    void move_file(const std::string &src, const std::string &dest, const FileInfo &fi);
    void endeaden(const std::string &filepath);

    SystemInfo getSysinfo();

    Agent();
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
    Agent(const Agent&&) = delete;
    Agent& operator=(const Agent&&) = delete;

    // for testing
    friend class SecretAgent;

 public:
    explicit Agent(const AgentConfig &cfg);
    ~Agent();

    Cleaner& getCleaner();

    void setUavClient(AgentUAVCANClient *client);
    void setAdcsMgr(AdcsManager *mgr) {adcs = mgr;}

    void setMinFreeMb(int mb);
    void setMinFreePct(int pct);

    const std::string &getWorkdir() const {
        return workdir;
    }

    void setMaxQuery(int max);

    // SDK methods
    ResponseCode<AvailableFilesResponse> query_available(const std::string &topic);
    ResponseCode<SendFileResponse> send_file(const SendFileRequest &req);
    ResponseCode<FileInfo> retrieve_file(const RetrieveFileRequest &req);

    // SDK ADCS methods
    ResponseCode<AdcsResponse> adcs_get();
    ResponseCode<AdcsCommandResponse> adcs_command(const AdcsCommandRequest &req);

    // SDK TFRS methods
    ResponseCode<TfrsResponse> tfrs_get();

    // Collector methods
    ResponseCode<InfoResponse> collector_info(InfoRequest &req);  // model not defined const
    ResponseCode<TransferMeta> meta(const std::string &uuid);
    ResponseCode<PingResponse> ping();

    // version strings
    std::string getBuildVersion();
    std::string getReleaseVersion();
    std::string getApiVersion();
};
