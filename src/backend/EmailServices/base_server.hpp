#ifndef BASE_SERVER_HPP
#define BASE_SERVER_HPP

#include "parse_input.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/**
 * Base server abstract class. Starts server by creating socket and binding and listening to
 * specified port
 */
class BaseServer {
  public:
    const int max_num_fds = 5000;
    const int backlog = 256;
    int sockfd = -1;
    std::vector<pthread_t> threads;

    virtual void int_handler(int sig) = 0;

    virtual void server_loop() = 0;

    BaseServer() : threads(max_num_fds, -1) {}

    void start() {
        struct sockaddr_in servaddr;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            raise_error("Unable to create socket");
        } else if (debug_output) {
            print_info("Created socket");
        }

        const int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
            raise_error("setsockopt(SO_REUSEADDR) failed");

        bzero(&servaddr, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(portno);

        if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
            raise_error("Unable to bind socket");
        } else if (debug_output) {
            print_info("Binded socket");
        }

        if ((listen(sockfd, backlog)) != 0) {
            raise_error("Unable to listen");
        } else if (debug_output) {
            print_info("Server listening");
        }
    }
};

#endif
