/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Sunday Sep 05, 2021 19:23:03 NPT
 * License     : MIT
 */

#ifndef REQUEST_H

#define REQUEST_H

#include <stddef.h> // for size_t
#include "picohttpparser.h"

struct HTTPRequest {
    const char *method;
    const char *path;
    int minor_ver;
    struct phr_header headers[100];
    size_t method_len;
    size_t path_len;
    size_t num_headers;
};

enum ReqParserResult {
    REQ_PARSE_SUCCESS,
    REQ_PARSE_ERROR,
    REQ_IS_TOO_LONG,
};

typedef void (*res_handler_cb)(int, const char *, size_t);
void handle_request(int client_sock_fd, res_handler_cb send_res);

#endif /* REQUEST_H */

