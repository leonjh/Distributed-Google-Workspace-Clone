#ifndef STORAGE_SERVER_HPP
#define STORAGE_SERVER_HPP

#include "BaseServer.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

void *worker_func_wrapper(void *arg);

/**
 * SMTP Server implementation
 */
class StorageServer : public BaseServer {
  private:
    const std::string greeting_message = "220 Service ready (Author: Joseph Zhang / jzhang25)\r\n";
    const std::string unknown_cmd_message = "500 Syntax error, command unrecognized\r\n";
    const std::string unknown_params_message = "501 Syntax error in parameters\r\n";
    const std::string bad_sequence_message = "503 Bad sequence of commands\r\n";
    const std::string shutting_down_message =
        "221 localhost Service closing transmission channel\r\n";
    const std::string mailbox_not_found = "550 No such user here\r\n";
    const std::string ok_message = "250 OK\r\n";
    const std::string ready_to_receive = "354 Start mail input; end with <CRLF>.<CRLF>\r\n";

    struct session_state {
        bool reading_email_data; // currently processing a DATA command
        bool received_hello;     // already received HELO
        std::string sender, email;
        std::vector<std::string> recipients;
    };

    std::unordered_map<std::string, int> user_file_index;
    std::vector<std::string> files;
    std::vector<std::unique_ptr<std::mutex>> file_locks;

  public:
    StorageServer(int portno) : BaseServer{portno} {}

    /**
     * Thread worker function that takes care of reading messages from client
     */
    void *worker_func(void *input) {
        // Block sigint for all workers
        sigset_t thread_set;
        sigemptyset(&thread_set);
        sigaddset(&thread_set, SIGINT);
        pthread_sigmask(SIG_BLOCK, &thread_set, NULL);

        struct session_state tmp_state = {0, 0, {}, {}, {}};
        struct session_state *state = &tmp_state;

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
                    std::cout << s << std::endl;
                    // if (process(connfd, s, state)) // client said quit
                    // {
                    //     threads[connfd] = nullptr;
                    //     close(connfd);
                    //     return NULL;
                    // }
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
            // if (debug_output)
            // {
            //     debug_new_conn(connfd);
            //     debug_sent(connfd, greeting_message);
            // }
            std::cout << connfd << std::endl;
            pthread_create(&threads[*input_fd], NULL, &worker_func_wrapper, input_fd);
        }
    }

  private:
    // /**
    //  * Helper function to write responses to clients
    //  */
    // void write_response(int connfd, const std::string &msg)
    // {
    //     write(connfd, msg.c_str(), strlen(msg.c_str()));
    //     if (debug_output)
    //     {
    //         debug_sent(connfd, msg);
    //     }
    // }
};

extern StorageServer *server;

void *worker_func_wrapper(void *arg) {
    server->worker_func(arg);
    return NULL;
}

void int_handler_wrapper(int sig) { server->int_handler(sig); }

#endif
