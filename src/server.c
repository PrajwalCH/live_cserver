/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Wednesday Sep 01, 2021 08:57:25 NPT
 * License     : MIT
 */

#include "server.h"

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#include "debug.h"

#define BACKLOG 10

typedef struct HTTPRequest {
    char method[50];
    char path[50];
    char http_ver[50];
} HTTPRequest;

void log_request(HTTPRequest *req_obj)
{
#define GREEN_ON "\033[0;32m"
#define COLOR_OFF "\033[0m"
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    DEBUG_LOG(stdout, GREEN_ON"[%02d:%02d:%02d] "COLOR_OFF"%s %s\n", time_info->tm_hour % 12, time_info->tm_min, time_info->tm_sec, req_obj->method, req_obj->path);
}

void handle_response(int client_sock_fd)
{
    const char *res_buff = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nHello World";
    send(client_sock_fd, res_buff, strlen(res_buff), 0);
}

static HTTPRequest parse_request(char *req_buff)
{
    HTTPRequest req_obj = {
        .method = {0},
        .path = {0},
        .http_ver = {0},
    };

    bool is_method_extracted = false;
    bool is_path_extracted = false;
    int space_encounter = 0;
    int idx = 0;

    while (*req_buff != '\r') {
        if (*req_buff == ' ') {
            space_encounter += 1;

            if (space_encounter == 1)
                is_method_extracted = true;

            if (space_encounter == 2)
                is_path_extracted = true;

            idx = 0;
            req_buff++; // skip the space
        }

        if (space_encounter == 0 && !is_method_extracted)
            req_obj.method[idx] = *req_buff;

        if (space_encounter == 1 && !is_path_extracted)
            req_obj.path[idx] = *req_buff;

        if (space_encounter == 2 && is_path_extracted)
            req_obj.http_ver[idx] = *req_buff;

        idx++;
        req_buff++;
    }
    return req_obj;
}

typedef void (*res_handler_cb)(int);
void handle_request(int client_sock_fd, res_handler_cb send_res)
{
    char req_buff[4096] = {0};
    int bytes_recvd = recv(client_sock_fd, req_buff, sizeof(req_buff) - 1, 0);
    if (bytes_recvd < 0) {
        DEBUG_PERROR("fail to receive data");
        return;
    }
    DEBUG_LOG(stdout, "%s", req_buff);
    HTTPRequest req_obj = parse_request(req_buff);
    log_request(&req_obj);
    send_res(client_sock_fd);
}

static int init_socket(const char *port_num, const char *host_addr)
{
    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_addr = NULL,
        .ai_canonname = NULL,
        .ai_next = NULL
    };
    struct addrinfo *result,
                    *tmp_ai;
    int master_sock_fd = -1;
    int ret_value = getaddrinfo(host_addr, port_num, &hints, &result);

    if (ret_value != 0) {
        DEBUG_LOG(stderr, "getaddrinfo: %s\n", gai_strerror(ret_value));
        freeaddrinfo(result);
        return -1;
    }

    for (tmp_ai = result; tmp_ai != NULL; tmp_ai = tmp_ai->ai_next) {
        if ((master_sock_fd = socket(tmp_ai->ai_family, tmp_ai->ai_socktype, tmp_ai->ai_protocol)) == -1) {
            DEBUG_PERROR("fail to create socket");
            continue;
        }

        if (setsockopt(master_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
            DEBUG_PERROR("fail to set socket address as reusable");
            close(master_sock_fd);
            master_sock_fd = -1;
            continue;
        }

        if (bind(master_sock_fd, tmp_ai->ai_addr, tmp_ai->ai_addrlen) == -1) {
            DEBUG_PERROR("fail to bind socket");
            close(master_sock_fd);
            master_sock_fd = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (tmp_ai == NULL) {
        DEBUG_LOGLN(stderr, "live_cserver: cannot find valid local address");
        return -1;
    }
    if (listen(master_sock_fd, BACKLOG) != 0) {
        DEBUG_PERROR("fail to listen socket");
        close(master_sock_fd);
        return -1;
    }
    printf("Server listening on: http://localhost:%s\n", port_num);
    return master_sock_fd;
}

void start_server(ServerConfig server_config)
{
   int master_sock_fd = init_socket(server_config.port_num, server_config.host_addr);
   if (master_sock_fd == -1) return;
   int client_sock_fd = -1;
   struct sockaddr_in client_addr;

   while (1) {
       socklen_t client_addr_len = sizeof(struct sockaddr_in);
       if ((client_sock_fd = accept(master_sock_fd, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
           DEBUG_PERROR("fail to accept new connection");
           continue;
       }
       handle_request(client_sock_fd, handle_response);
       close(client_sock_fd);
   }
}

ServerConfig default_server_config(void)
{
    ServerConfig default_config = {
        .folder_path = {0},
        .port_num = DEFAULT_PORT_NUM,
        .host_addr = DEFAULT_HOST_ADDR,
        .verbose_flag = DEFAULT_VERBOSE_FLAG,
        .help_flag = 0,
    };
    return default_config;
}
