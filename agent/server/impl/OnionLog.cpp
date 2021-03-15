/**
 * OnionLog.cpp
 *
 * Shim for onion logging
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "onion/log.h"

#include "OnionLog.h"
#include "Log.h"

extern "C" {

void agent_onion_log(onion_log_level level, const char *filename, int line, const char *fmt, ...) {
    va_list ap;
    int size;
    char *buf = NULL;

    va_start(ap, fmt);
    size = vsnprintf(buf, 0, fmt, ap);
    va_end(ap);

    if (size < 0) {
        return;
    }
    ++size;
    buf = static_cast<char *>(malloc(size));

    va_start(ap, fmt);
    vsnprintf(buf, size, fmt, ap);
    va_end(ap);

    switch (level) {
        case O_DEBUG0:
        case O_DEBUG:
            Log::debug("?", buf);
            break;
        case O_INFO:
            Log::info("?", buf);
            break;
        case O_WARNING:
            Log::warn("?", buf);
            break;
        case O_ERROR:
            Log::error("?", buf);
            break;
        default:
            break;
    }
    free(buf);
}

}  // extern "C"
