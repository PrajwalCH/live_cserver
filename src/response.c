/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Wednesday Sep 08, 2021 11:51:59 NPT
 * License     : MIT
 */

#include "response.h"

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

    const char fmt[] = "%s %zu %s"CRLF"Server: live_cserver"CRLF"Content-Type: %s"CRLF"Content-Length: %zu"CRLF"Connection: close"CRLF"Date: %s"CRLF"%s";
    int num_bytes_written = snprintf(buff, buff_size, fmt, res_header->protocol_ver, res_header->status_code, status_txt, content_type, res_header->content_length, res_header->date, res_body);
    if (num_bytes_written < 0 || num_bytes_written == 0) {
        fprintf(stderr, "failed to write response header on buffer\n");
        memset(buff, 0, buff_size);
        return false;
    }
    return true;
}

static void response_send_file(int client_sock_fd, struct ResponseHeader *res_header, FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long chunk_size = ftell(fp);
    rewind(fp);

    res_header->content_length = 0 + chunk_size;

    char res_buff[RES_BUFF_SIZE + 1] = {0};
    char *file_buff = malloc(sizeof(char) * chunk_size + 1);

    memset(file_buff, 0, chunk_size + 1);

    if (fread(file_buff, 1, chunk_size, fp) != res_header->content_length) {
        DEBUG_LOGLN(stdout, "Unable to read file");
        fclose(fp);
        free(file_buff);
        return;
    }
    fclose(fp);

    if (!construct_response(res_header, res_buff, RES_BUFF_SIZE, file_buff))
        return;

    send(client_sock_fd, res_buff, RES_BUFF_SIZE, 0);
    free(file_buff);
}

static void response_send(int client_sock_fd, struct ResponseHeader *res_header, const char *res_body)
{
    char res_buff[RES_BUFF_SIZE + 1] = {0};
    res_header->content_length = strlen(res_body);
    if (!construct_response(res_header, res_buff, RES_BUFF_SIZE, res_body))
        return;

    DEBUG_LOG(stdout, "res_buff: \n%s\n", res_buff);
    send(client_sock_fd, res_buff, RES_BUFF_SIZE, 0);
}

static inline void response_set_content_type(enum MIMEType mime_type, struct ResponseHeader *res_header)
{
    res_header->content_type = mime_type;
}

static inline void response_set_status_code(enum StatusCode code, struct ResponseHeader *res_header)
{
    res_header->status_code = code;
}

static enum MIMEType get_content_type_from_fileext(const char *file_pathname)
{
    char *last_slash = strrchr(file_pathname, '/');
    char *file_ext = strrchr(last_slash, '.');

    if (strcmp(file_ext, ".html") == 0)
        return MIMEType_html;
    if (strcmp(file_ext, ".css") == 0)
        return MIMEType_css;
    if (strcmp(file_ext, ".js") == 0)
        return MIMEType_js;

    return MIMEType_default;
}

static void construct_file_pathname(char *file_pathname_buff, const char *rootdir_pathname, char *url_pathname, size_t url_pathname_len)
{
    char *last_slash = memrchr(url_pathname, '/', url_pathname_len);
    const char *fmt = "%s%.*s";
    struct stat stat_buff;

    // remove trailing slash
    if (last_slash != NULL && last_slash != url_pathname && last_slash[1] == '\r')
        last_slash[0] = '\0';

    snprintf(file_pathname_buff, FILE_PATHNAME_BUFF_SIZE, fmt, rootdir_pathname, url_pathname_len, url_pathname);

    if (stat(file_pathname_buff, &stat_buff) == 0 && S_ISDIR(stat_buff.st_mode)) {
        if (url_pathname_len == 1)
            fmt = "%s%.*s%s";
        else
            fmt = "%s%.*s/%s"; // add '/' explicitly after dirname if filename is not mention on the url path
        snprintf(file_pathname_buff, FILE_PATHNAME_BUFF_SIZE, fmt, rootdir_pathname, url_pathname_len, url_pathname, DEFAULT_HTML_FILENAME);
    }
}

static struct ResponseHeader init_ResponseHeader_obj(void)
{
    time_t raw_time = time(NULL);
    struct ResponseHeader res_header = {
        .protocol_ver = "HTTP/1.1",
        .status_code = StatusCode_ok,
        .content_type = MIMEType_default,
        .content_length = 0,
        .date = ctime(&raw_time),
        .connection = "Keep-Alive",
    };
    return res_header;
}

void handle_response(int client_sock_fd, const char *rootdir_pathname, char *url_pathname, size_t url_pathname_len)
{
    struct ResponseHeader res_header = init_ResponseHeader_obj();
    char file_pathname[FILE_PATHNAME_BUFF_SIZE + 1] = {0};
    construct_file_pathname(file_pathname, rootdir_pathname, url_pathname, url_pathname_len);
    DEBUG_LOG(stdout, "file: %s\n", file_pathname);
    FILE *fp = fopen(file_pathname, "r");

    if (fp == NULL) {
        response_set_status_code(StatusCode_not_found, &res_header);
        response_set_content_type(MIMEType_html, &res_header);
        response_send(client_sock_fd, &res_header, "<h2>404 Not Found</h2>");
        return;
    }

    response_set_content_type(get_content_type_from_fileext(file_pathname), &res_header);
    response_send_file(client_sock_fd, &res_header, fp);
}
