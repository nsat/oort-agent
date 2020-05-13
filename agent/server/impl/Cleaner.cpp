/**
 * Cleaner.cpp
 *
 * Agent cleanup task
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include "Cleaner.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <system_error>  // NOLINT(build/c++11)
#include <utility>
#include <vector>

#include "Agent.h"
#include "Files.h"
#include "Log.h"
#include "Utils.h"

using namespace std;

Cleaner::Cleaner(const Agent *agent) : m_agent(agent) {
    if (sem_init(&runningSem, 0, 1) != 0) {
        throw system_error(errno, generic_category(), "initializing semaphore");
    }
    workerRunning = false;
}

void Cleaner::setCleanupInterval(int interval) {
    Log::info("setting cleanup interval to ? seconds", interval);
    m_cleanupInterval = interval;
}

void Cleaner::setMaxAge(int age) {
    Log::info("setting cleanup maximum age to ? seconds", age);
    m_maxAge = age;
}

void Cleaner::doCleanup() {
    Log::debug("run Cleaner::doCleanup");
    for (string dir : {m_agent->transfer_dir, m_agent->upload_dir, m_agent->upgrade_dir}) {
        Log::info("Cleaning ?", dir);
        cleanOldFiles(dir);
    }
}

void Cleaner::cleanupTask() {
    Log::setThreadName("cleanup");
    Log::info("cleanup task starting; interval ?", m_cleanupInterval);
    struct timespec ts = {0, 0};
    while (workerRunning) {
        chrono::system_clock::time_point next_run(
               chrono::system_clock::now() + chrono::seconds(m_cleanupInterval));

        ts.tv_sec = chrono::duration_cast<chrono::seconds>(
            next_run.time_since_epoch()).count();

        int status = sem_timedwait(&runningSem, &ts);
        if (workerRunning && status == -1 && errno == ETIMEDOUT) {
            doCleanup();
        }
    }
    Log::info("cleanup task exiting");
}

void Cleaner::cleanOldFiles(const string &dir) {
  auto flist = Files::oldFiles(dir, m_maxAge);
    for (auto f = flist.begin(); f != flist.end(); ++f) {
        // pass 1: remove old ".meta" files
        if (ends_with(*f, m_agent->META_EXT)) {
            Log::warn("removing old meta file ?", *f);
            unlink(f->c_str());
        }
    }
    for (auto f = flist.begin(); f != flist.end(); ++f) {
        // pass 2: remove old data files if they have no associated "meta" file
        if (ends_with(*f, m_agent->DATA_EXT)
                && access(m_agent->meta_file(*f).c_str(), R_OK) != 0) {
            Log::warn("removing orphaned file ?", *f);
            unlink(f->c_str());
        }
    }
}

void Cleaner::scheduleCleanup() {
    workerRunning = true;
    sem_wait(&runningSem);
    thread worker(&Cleaner::cleanupTask, this);
    ostringstream s("");
    s << worker.get_id();
    Log::info("cleanup worker thread id = ?", s.str());
    cleanupWorker = std::move(worker);
}

void Cleaner::stopWorker() {
    workerRunning = false;
    sem_post(&runningSem);
    cleanupWorker.join();
}
