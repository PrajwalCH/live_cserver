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
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "debug.h"
#include "request.h"
#include "response.h"

#define BACKLOG 10

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

void start_server(struct ServerConfig server_config)
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

struct ServerConfig default_server_config(void)
{
    struct ServerConfig default_config = {
        .folder_path = {0},
        .port_num = DEFAULT_PORT_NUM,
        .host_addr = DEFAULT_HOST_ADDR,
        .verbose_flag = DEFAULT_VERBOSE_FLAG,
        .help_flag = 0,
    };
    return default_config;
}
