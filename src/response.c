/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Wednesday Sep 08, 2021 11:51:59 NPT
 * License     : MIT
 */

#include "response.h"

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#include "debug.h"

#define CRLF "\r\n"
#define RES_BUFF_SIZE 4095
#define RES_HEADER_BUFF_SIZE 100
#define FILE_PATHNAME_BUFF_SIZE 255
#define DEFAULT_HTML_FILENAME "index.html"

static const char *get_status_txt_str(enum StatusCode status_code)
{
    const char *status_txt = "NOT FOUND";

    switch (status_code) {
        case StatusCode_ok:
            status_txt = "OK";
            break;
        case StatusCode_not_found:
            status_txt = "Not Found";
            break;
        case StatusCode_internal_server_error:
            status_txt = "Internal Server Error";
            break;
        default:
            break;
    }
    return status_txt;
}

static bool construct_response(struct ResponseHeader *res_header, char *buff, size_t buff_size, const char *res_body)
{
    static char mime_types[][255] = {
        "application/octet-stream",
        "text/css",
        "image/gif",
        "text/htm",
        "text/html",
        "image/vnd.microsoft.icon",
        "image/jpg",
        "image/jpeg",
        "text/js",
        "application/json",
        "font/otf",
        "image/png",
        "image/svg+xml",
        "text/plain"
    };

    const char *status_txt = get_status_txt_str(res_header->status_code);
    const char *content_type = mime_types[res_header->content_type];

    const char fmt[] = "%s %zu %s"CRLF"Content-Type: %s"CRLF"Content-Length: %zu"CRLF"Date: %s"CRLF"%s";
    int num_bytes_written = snprintf(buff, buff_size, fmt, res_header->protocol_ver, res_header->status_code, status_txt, content_type, res_header->content_length, res_header->date, res_body);
    if (num_bytes_written < 0 || num_bytes_written == 0) {
        fprintf(stderr, "failed to write response header on buffer\n");
        memset(buff, 0, buff_size);
        return false;
    }
    return true;
}

static struct HTTPResponse init_HTTPResponse_obj(void)
{
    time_t raw_time = time(NULL);
    struct HTTPResponse res_obj = {
        .protocol_ver = "HTTP/1.1",
        .status_code = StatusCode_ok,
        .content_type = MIMEType_default,
        .content_length = 0,
        .date = ctime(&raw_time),
        .connection = "Keep-Alive",
    };
    return res_obj;
}

void handle_response(int client_sock_fd, const char *path, size_t path_len)
{
    struct HTTPResponse res_obj = init_HTTPResponse_obj();
    
    char res_header_buff[RES_HEADER_BUFF_SIZE + 1] = {0};
    char res_body_buff[4096] = {0};
    char res_buff[RES_BUFF_SIZE + 1] = {0};

    if (strncmp(path, "/", path_len) == 0) {
        res_obj.content_type = MIMEType_html;
        if (!construct_response_header(&res_obj, res_header_buff, RES_HEADER_BUFF_SIZE, strlen(res_body_buff)))
            return;
        snprintf(res_buff, RES_BUFF_SIZE, "%s%s", res_header_buff, res_body_buff);
        DEBUG_LOG(stdout, "%s\n", res_buff);
        send(client_sock_fd, res_buff, strlen(res_buff), 0);
    }
}
