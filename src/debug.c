/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Thursday Sep 02, 2021 17:40:35 NPT
 * License     : MIT
 */

#include "debug.h"

void dbg_log(FILE *stream, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
}
