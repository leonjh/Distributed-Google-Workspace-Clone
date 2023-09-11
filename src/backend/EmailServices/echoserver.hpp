#ifndef ECHO_SERVER_HPP
#define ECHO_SERVER_HPP

#include "base_server.hpp"
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
#include <unordered_map>
#include <vector>

void *worker_func_wrapper(void *arg);

/**
 * Echo Server implementation
 */
class EchoServer : public BaseServer {
  public:
    const std::string greeting_message = "+OK Server ready (Author: Joseph Zhang / jzhang25)\r\n";
    const std::string unknown_cmd_message = "-ERR Unknown command\r\n";
    const std::string shutting_down_message = "-ERR Server shutting down\r\n";

    /**
     * Sigint handler. Iterates through existing threads and sends a shutting down message to
     * each client and closes and frees file descriptor.
     */
    void int_handler(int sig) {
        for (int i = 0; i < threads.size(); ++i) {
            if (threads[i] == -1) {
                continue;
            }
            pthread_cancel(threads[i]);
            pthread_join(threads[i], NULL);
            write(i, shutting_down_message.c_str(), strlen(shutting_down_message.c_str()));
            close(i);
        }
        if (sockfd != -1) {
            close(sockfd);
        }
        exit(EXIT_SUCCESS);
    }

    /**
     * Process command received from client
     */
    bool process(int connfd, const std::string &cmd) {
        if (debug_output) {
            debug_received(connfd, cmd);
        }
        std::string stripped = cmd.substr(0, cmd.size() - 2);
        for (int i = 0; i < stripped.size() && i < 4; ++i) {
            stripped[i] = toupper(stripped[i]);
        }
        if (stripped.size() >= 4 && stripped.substr(0, 4) == "ECHO") {
            std::string response = "+OK ";
            if (stripped.size() >= 6) {
                response += stripped.substr(5);
            }
            response += "\r\n";
            write(connfd, response.c_str(), strlen(response.c_str()));
            if (debug_output) {
                debug_sent(connfd, response);
            }
        } else if (stripped == "QUIT") {
            std::string response = "+OK Goodbye!\r\n";
            write(connfd, response.c_str(), strlen(response.c_str()));
            if (debug_output) {
                debug_sent(connfd, response);
                debug_closed_conn(connfd);
            }
            return 1;
        } else // Unknown command
        {
            write(connfd, unknown_cmd_message.c_str(), strlen(unknown_cmd_message.c_str()));
            if (debug_output) {
                debug_sent(connfd, unknown_cmd_message);
            }
        }
        return 0;
    }

    /**
     * Thread worker function that takes care of reading messages from client
     */
    void *worker_func(void *input) {
        // Block sigint for all workers
        sigset_t thread_set;
        sigemptyset(&thread_set);
        sigaddset(&thread_set, SIGINT);
        pthread_sigmask(SIG_BLOCK, &thread_set, NULL);

        int *connfd = (int *)input;
        std::vector<std::string>
            processed; // holds all full commands in buffer (last command may be unfinished)
        char buffer[1024];
        int num_bytes;
        while ((num_bytes = read(*connfd, buffer, sizeof(buffer))) != -1) {
            if (processed.empty()) {
                processed.push_back(std::string());
            }
            for (int i = 0; i < num_bytes; ++i) {
                if (processed.back().size() && processed.back().back() == '\n') {
                    processed.push_back(std::string());
                }
                processed.back().push_back(buffer[i]);
            }
            std::string save;
            for (auto &s : processed) {
                if (s.back() == '\n') {
                    if (process(*connfd, s)) // client said quit
                    {
                        threads[*connfd] = -1;
                        close(*connfd);
                        delete connfd;
                        return NULL;
                    }
                } else {
                    // s must be a string at the end of processed
                    save = s;
                }
            }
            if (save.size()) {
                processed = {save};
            } else {
                processed = {};
            }
        }
        delete connfd;
        return NULL;
    }

    /**
     * Main server loop that accepts connections, writes greeting messages, and creates workers
     * for each new connection
     */
    void server_loop() {
        struct sockaddr_in cli;
        socklen_t len = sizeof(sockaddr_in);
        int connfd;
        while ((connfd = accept(sockfd, (struct sockaddr *)&cli, &len)) >= 0) {
            int *input_fd = new int(connfd);
            write(connfd, greeting_message.c_str(), strlen(greeting_message.c_str()));
            if (debug_output) {
                debug_new_conn(connfd);
                debug_sent(connfd, greeting_message);
            }
            pthread_create(&threads[*input_fd], NULL, &worker_func_wrapper, input_fd);
        }
    }
};

extern EchoServer *server;

void *worker_func_wrapper(void *arg) {
    server->worker_func(arg);
    return NULL;
}

void int_handler_wrapper(int sig) { server->int_handler(sig); }

#endif
