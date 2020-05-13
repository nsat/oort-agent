/**
 * Cleaner.h
 *
 * Agent cleanup task.  
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include <semaphore.h>
#include <atomic>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>

class Agent;

/**
 * \brief Agent cleanup task.
 * 
 * The Cleaner is responsible for scanning the Agent's working directories
 * and deleting files that are too old.  This is only an internal helper
 * for the agent and is not intended to run on its own.
 */
class Cleaner {
    const Agent *m_agent;

    int m_cleanupInterval = 3600;
    int m_maxAge = 86400;
    std::thread cleanupWorker;
    std::atomic<bool> workerRunning;
    sem_t runningSem;

    Cleaner();

    void doCleanup();
    void cleanupTask();
    void cleanOldFiles(const std::string &dir);

 public:
    explicit Cleaner(const Agent *agent);

    void setCleanupInterval(int interval);
    void setMaxAge(int age);

    void scheduleCleanup();
    void stopWorker();
};
