/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Wednesday Sep 08, 2021 11:52:14 NPT
 * License     : MIT
 */

#ifndef RESPONSE_H

#define RESPONSE_H

#include <stddef.h> // for size_t

enum StatusCode {
    StatusCode_ok = 200,
    StatusCode_not_found = 404,
    StatusCode_internal_server_error = 500
};

enum MIMEType {
    MIMEType_default,
    MIMEType_css,
    MIMEType_gif,
    MIMEType_htm,
    MIMEType_html,
    MIMEType_ico,
    MIMEType_jpg,
    MIMEType_jpeg,
    MIMEType_js,
    MIMEType_json,
    MIMEType_otf,
    MIMEType_png,
    MIMEType_svg,
    MIMEType_txt,
};

struct ResponseHeader {
    const char *protocol_ver;
    enum StatusCode status_code;
    enum MIMEType content_type;
    size_t content_length;
    const char *date;
    const char *connection;
};

void handle_response(int client_sock_fd, const char *rootdir_pathname, char *url_pathname, size_t url_pathname_len);

#endif /* RESPONSE_H */

