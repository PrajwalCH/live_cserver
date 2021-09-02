/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Thursday Sep 02, 2021 17:41:11 NPT
 * License     : MIT
 */

#ifndef DEBUG_H

#define DEBUG_H

#include <stdio.h>

#define DEBUG_LOG(stream, fmt, ...) \
    do { if (DEBUG) dbg_log(stream, fmt, __VA_ARGS__); } while (0)

#define DEBUG_LOGLN(stream, ...) \
    DEBUG_LOG(stream, "%s\n", __VA_ARGS__);

#define DEBUG_PERROR(msg) \
    do { if(DEBUG) perror(msg); } while (0)

void dbg_log(FILE *stream, const char *fmt, ...);

#endif /* DEBUG_H */

