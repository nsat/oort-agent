/**
 * main-api-server.cpp
 *
 * OORT Agent main()
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#ifdef __linux__
#include <signal.h>
#include <unistd.h>
#include <vector>
#endif

#include "onion/onion.hpp"
#include "onion/url.hpp"
#include "onion/log.h"

#include "Agent.h"
#include "Config.h"
#include "CollectorApiImpl.h"
#include "SdkApiImpl.h"
#include "Log.h"
#include "OnionLog.h"

Onion::Onion *server = NULL;

#ifdef __linux__
static void sigHandler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGHUP:
        default:
            Log::warn("shutting down on signal");
            if (server != NULL) {
                Log::warn("stopping listener");
                server->listenStop();
            }
            break;
    }
}

static void setUpUnixSignals(std::vector<int> quitSignals) {
    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}
#endif

int main(int argc, char * const argv[]) {
#ifdef __linux__
    std::vector<int> sigs{SIGQUIT, SIGINT, SIGTERM, SIGHUP};
    setUpUnixSignals(sigs);
#endif

    // set onion library to use our logger
    onion_log = agent_onion_log;

    AgentConfig config;

    if (!config.parseOptions(argc, argv)) {
        config.usage(argv[0]);
        exit(1);
    }

    Agent agent(config);

    Log::info("Workdir = ?",  agent.getWorkdir());
    Log::info("starting up");

    server = new Onion::Onion(O_POLL | O_NO_SIGTERM);
    server->setMaxPostSize(8000);
    server->setMaxThreads(4);
    server->setPort(config.getPort());
    Onion::Url url(server);

    CollectorApiImpl collector(&agent);
    url.add("^collector/", std::move(collector));

    SdkApiImpl sdk(&agent);
    url.add("^sdk/", std::move(sdk));

    url.add("^.*", "Method not implemented", HTTP_NOT_IMPLEMENTED);

    Log::info("starting listener on port ?", config.getPort());
    server->listen();
    Log::info("listener stopped");
    delete server;
}
