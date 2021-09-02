/*
 * Author      : Prajwal Chapagain <prajjwal2058@gmail.com>
 * Date        : Tuesday Aug 31, 2021 15:46:29 NPT
 * License     : MIT
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

void print_usage(FILE *stream, const char *program_name)
{
    fprintf(stream,
            "Usage: %s [options] <folder_path>\n"
            "\nOptions:\n"
            "--port, -p <number> \t default: %s\n"
            "--host, -h <address> \t default: %s\n"
            "--verbose \t\t enable logging (default)\n"
            "--slient \t\t disable logging\n"
            "--help\n"
            "\nExample:\n"
            "%s --port=8080 --slient hello_world\n",
            program_name, DEFAULT_PORT_NUM, DEFAULT_HOST_ADDR, program_name);
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
                memcpy(server_config.port_num, optarg, MAX_PORT_NUM_LEN);
                break;
            case 'h':
                memcpy(server_config.host_addr, optarg, MAX_HOST_ADDR_LEN);
                break;
            case '?':
            default:
                break;
        }
    }

    if ((optind < argc) && (strnlen(argv[optind], MAX_FOLDER_PATH_LEN) > 0))
        memcpy(server_config.folder_path, argv[optind], MAX_FOLDER_PATH_LEN);
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

    if (strnlen(server_config.folder_path, MAX_FOLDER_PATH_LEN) < 1) {
        fprintf(stderr, "Which folder to serve?\n");
        print_usage(stderr, program_name);
        exit(EXIT_FAILURE);
    }
    start_server(server_config);
    return 0;
}

