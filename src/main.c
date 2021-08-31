/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Tuesday Aug 31, 2021 15:46:29 NPT
 * License     : MIT
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 2058
#define DEFUALT_HOST "127.0.0.0"

typedef struct ServerConfig {
    int port_num;
    char host_addr[256];
    int verbose_flag;
    int help_flag;
    char folder_to_serve[100];
} ServerConfig;

void print_usage(FILE *stream, char *program_name)
{
    fprintf(stream,
            "Usage: %s [options] <folder_to_serve>\n"
            "\nOptions:\n"
            "--port, -p <number> \t default: %d\n"
            "--host, -h <address> \t default: %s\n"
            "--verbose \t\t enable logging (default)\n"
            "--slient \t\t disable logging\n"
            "--help\n"
            "\nExample:\n"
            "%s --port=8080 --slient hello_world\n",
            program_name, DEFAULT_PORT, DEFUALT_HOST, program_name);
}

ServerConfig default_server_config(void)
{
    ServerConfig default_config = {
        .port_num = DEFAULT_PORT,
        .host_addr = DEFUALT_HOST,
        .verbose_flag = 1,
        .help_flag = 0,
        .folder_to_serve = {0}
    };
    return default_config;
}

static ServerConfig parse_args(int argc, char **argv)
{
    ServerConfig server_config = default_server_config();
    struct option options[] = {
        {"port", required_argument, NULL, 'p'},
        {"host", required_argument, NULL, 'h'},
        {"verbose", no_argument, &(server_config.verbose_flag), 1},
        {"slient", no_argument, &(server_config.verbose_flag), 0},
        {"help", no_argument, &(server_config.help_flag), 1 },
        {NULL, 0, NULL, 0 },
    };

    int opt_ch = 0;
    int opt_idx = 0;
    
    while (1) {
        opt_ch = getopt_long(argc, argv, "p:h:h", options, &opt_idx);
        if (opt_ch == -1) break;

        switch (opt_ch) {
            case 'p':
                server_config.port_num = atoi(optarg);
                break;
            case 'h':
                memcpy(server_config.host_addr, optarg, sizeof(server_config.host_addr));
                break;
            case '?':
            default:
                break;
        }
    }

    if ((optind < argc) && (strlen(argv[optind]) > 0))
        memcpy(server_config.folder_to_serve, argv[optind], sizeof(server_config.folder_to_serve));
    return server_config;
}

int main(int argc, char **argv)
{
    ServerConfig server_config = parse_args(argc, argv);
    char *program_name = argv[0]; // require to print on usage/help information

    if (server_config.help_flag) {
        print_usage(stdout, program_name);
        exit(EXIT_SUCCESS);
    }

    if (strlen(server_config.folder_to_serve) < 1) {
        fprintf(stderr, "Which folder to serve?\n");
        print_usage(stderr, program_name);
        exit(EXIT_FAILURE);
    }
    printf("port_num: %i\n"
           "host_addr: %s\n"
           "verbose: %i\n"
           "folder_to_serve: %s",
           server_config.port_num, server_config.host_addr, server_config.verbose_flag, server_config.folder_to_serve);
    return 0;
}

