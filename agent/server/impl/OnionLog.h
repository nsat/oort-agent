/**
 * OnionLog.h
 *
 * Shim for onion logging
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#include "onion/log.h"

extern "C" {
    typedef void (onion_log_t)(onion_log_level level,
        const char *filename, int line, const char *fmt, ...);

    void agent_onion_log(onion_log_level level,
        const char *filename, int line, const char *fmt, ...);
}
