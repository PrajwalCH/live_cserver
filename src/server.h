/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Wednesday Sep 01, 2021 10:58:56 NPT
 * License     : MIT
 */

#ifndef SERVER_H

#define SERVER_H

#define MAX_PORT_NUM_LEN 1025
#define MAX_HOST_ADDR_LEN 255
#define MAX_FOLDER_PATH_LEN 1025
#define NULL_BYTE 1

#define DEFAULT_PORT_NUM "8000"
#define DEFAULT_HOST_ADDR "127.0.0.1"
#define DEFAULT_VERBOSE_FLAG 1

typedef struct ServerConfig {
    char folder_path[MAX_FOLDER_PATH_LEN + NULL_BYTE];
    char port_num[MAX_PORT_NUM_LEN + NULL_BYTE];
    char host_addr[MAX_HOST_ADDR_LEN + NULL_BYTE];
    int verbose_flag;
    int help_flag;
} ServerConfig;

void start_server(ServerConfig server_config);
ServerConfig default_server_config(void);

#endif /* SERVER_H */

