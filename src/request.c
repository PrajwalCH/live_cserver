/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Sunday Sep 05, 2021 19:22:55 NPT
 * License     : MIT
 */

#include "request.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#include "debug.h"

void log_request(struct HTTPRequest *req_obj)
{
#define GREEN_ON "\033[0;32m"
#define COLOR_OFF "\033[0m"
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    DEBUG_LOG(stdout,
              GREEN_ON"[%02d:%02d:%02d] "COLOR_OFF"%.*s %.*s\n",
              time_info->tm_hour % 12, time_info->tm_min, time_info->tm_sec,
              req_obj->method_len, req_obj->method, req_obj->path_len, req_obj->path);
}

static enum ReqParserResult parse_request(const char *req_buff, size_t req_buff_size, ssize_t bytes_recvd, struct HTTPRequest *req_obj)
{
    size_t req_buff_len = 0 + bytes_recvd;
    int phr_retv = phr_parse_request(req_buff, req_buff_len,
                                     &(req_obj->method), &(req_obj->method_len),
                                     &(req_obj->path), &(req_obj->path_len),
                                     &(req_obj->minor_ver),
                                     req_obj->headers, &(req_obj->num_headers),
                                     0);
    if (phr_retv > 0)
        return REQ_PARSE_SUCCESS;
    if (phr_retv == -1)
        return REQ_PARSE_ERROR;
    if (req_buff_len == req_buff_size)
        return REQ_IS_TOO_LONG;
    return REQ_PARSE_SUCCESS;
}

static struct HTTPRequest init_HTTPRequest_obj(void)
{
    struct HTTPRequest req_obj = {
        .method = NULL,
        .path = NULL,
        .minor_ver = 0,
        .headers = {0},
        .method_len = 0,
        .path_len = 0,
        .num_headers = 0
    };
    req_obj.num_headers = sizeof(req_obj.headers) / sizeof(req_obj.headers[0]);
    return req_obj;
}

void handle_request(int client_sock_fd, res_handler_cb send_res)
{
    char req_buff[4096] = {0};
    ssize_t bytes_recvd = bytes_recvd = recv(client_sock_fd, req_buff, sizeof(req_buff) - 1, 0);
    if (bytes_recvd < 0) {
        DEBUG_PERROR("fail to receive data");
        return;
    }
    DEBUG_LOG(stdout, "%s\n", req_buff);
    struct HTTPRequest req_obj = init_HTTPRequest_obj();
    enum ReqParserResult req_parser_result = parse_request(req_buff, sizeof(req_buff), bytes_recvd, &req_obj);
    if (req_parser_result == REQ_PARSE_ERROR) {
        DEBUG_LOGLN(stderr, "Request parse error");
        return;
    }

    if (req_parser_result == REQ_IS_TOO_LONG) {
        DEBUG_LOGLN(stderr, "Request is too long");
        return;
    }
    log_request(&req_obj);
    send_res(client_sock_fd, req_obj.path, req_obj.path_len);
}
