#ifndef SMTP_SERVER_HPP
#define SMTP_SERVER_HPP

#include "base_server.hpp"
#include "parse_input.h"
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
namespace fs = std::filesystem;

void *worker_func_wrapper(void *arg);

/**
 * SMTP Server implementation
 */
class SMTPServer : public BaseServer {
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
    /**
     * Initialize file locks
     */
    SMTPServer() {
        int index = 0;
        for (const auto &entry : fs::directory_iterator(directory)) {
            std::string path = entry.path();
            std::string filename = path.substr(path.find_last_of("/") + 1);
            if (filename == "mqueue") {
                user_file_index[".mqueue"] = index;
            } else {
                user_file_index[filename.substr(0, filename.find("."))] = index;
            }
            files.push_back(path);
            file_locks.push_back(std::make_unique<std::mutex>());
            index++;
        }
    }

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
            if (debug_output) {
                debug_sent(i, shutting_down_message);
            }
            close(i);
        }
        if (sockfd != -1) {
            close(sockfd);
        }
        exit(EXIT_SUCCESS);
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
                    if (process(*connfd, s, state)) // client said quit
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

  private:
    /**
     * Helper function to write responses to clients
     */
    void write_response(int connfd, const std::string &msg) {
        write(connfd, msg.c_str(), strlen(msg.c_str()));
        if (debug_output) {
            debug_sent(connfd, msg);
        }
    }

    /**
     * Get email address from command or return "" if invalid
     */
    std::string get_email(const std::string &cmd, int begin_position) {
        int start = -1, end = -1;
        for (int i = begin_position; i < cmd.size(); ++i) {
            if (cmd[i] == '<' && start == -1) {
                start = i;
            } else if (cmd[i] == '>') {
                end = i;
                break;
            }
        }

        std::string email = "";
        if (end > start && start != -1) {
            email = cmd.substr(start + 1, end - start - 1);
        }
        int pos = email.find("@");
        if (pos == -1 || email.substr(0, pos).size() == 0 || email.substr(pos + 1).size() == 0) {
            email = "";
        }
        return email;
    }

    /**
     * Reset state
     */
    void reset_state(struct session_state *state) {
        state->recipients = {};
        state->sender = "";
        state->email = "";
    }

    /**
     * Write email to each mbox of recipients
     */
    void write_email_to_files(struct session_state *state) {
        for (auto &recipient : state->recipients) {
            int at_pos = recipient.find("@");
            int file_pos;
            bool non_localhost = 0;
            if (recipient.substr(at_pos + 1) == "localhost") {
                file_pos = user_file_index[recipient.substr(0, at_pos)];
            } else {
                non_localhost = 1;
                file_pos = user_file_index[".mqueue"];
            }
            std::lock_guard<std::mutex> lock(*file_locks[file_pos]); // lock file on thread level
            FILE *file = fopen(files[file_pos].c_str(), "a");
            if (file == NULL) {
                perror("fopen(): ");
                exit(EXIT_FAILURE);
            }
            int fd = fileno(file);
            if (fd < 0) {
                perror("fileno(): ");
            }
            if (flock(fd, LOCK_EX) < 0) // lock file on process level (blocks)
            {
                perror("flock(): ");
            }
            time_t curr_time =
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::string to_write;
            if (non_localhost) {
                to_write += "To <" + recipient + ">\r\n";
            }
            to_write += "From <" + state->sender + "> " + std::ctime(&curr_time) + state->email;
            if (non_localhost) {
                to_write += ".\r\n";
            }
            if (fprintf(file, to_write.c_str(), sizeof(char) * to_write.size()) < 0) {
                perror("fprintf(): ");
            }
            if (flock(fd, LOCK_UN) < 0) {
                perror("flock(): ");
            }
            fclose(file);
        }
    }

    /**
     * Process command received from clientpos
     */
    bool process(int connfd, const std::string &cmd, struct session_state *state) {
        if (debug_output) {
            debug_received(connfd, cmd);
        }

        if (state->reading_email_data) {
            if (cmd == ".\r\n") {
                write_email_to_files(state);
                state->reading_email_data = 0;
                reset_state(state);
                write_response(connfd, ok_message);
            } else {
                state->email += cmd;
            }
        } else {
            std::string stripped = cmd.substr(0, cmd.size() - 2);
            for (int i = 0; i < stripped.size() && stripped[i] != ':'; ++i) {
                if (isalpha(stripped[i])) {
                    stripped[i] = toupper(stripped[i]);
                }
            }

            if (stripped.empty()) {
                return 0;
            }

            if (stripped.size() < 4) {
                write_response(connfd, unknown_cmd_message);
            } else if (stripped.substr(0, 4) == "HELO") {
                if (stripped.size() <= 5) // no domain
                {
                    write_response(connfd, unknown_params_message);
                    return 0;
                }
                state->received_hello = 1;
                if (state->sender.size() == 0 && state->recipients.size() == 0 &&
                    state->email.size() == 0) {
                    write_response(connfd, "250 localhost\r\n");
                } else {
                    write_response(connfd, bad_sequence_message);
                }
            } else if (stripped.size() >= 10 && stripped.substr(0, 10) == "MAIL FROM:") {
                if (!state->received_hello) {
                    write_response(connfd, bad_sequence_message);
                    return 0;
                }

                std::string email = get_email(stripped, 10);
                if (email == "") {
                    write_response(connfd, unknown_params_message);
                } else {
                    state->sender = email;
                    write_response(connfd, ok_message);
                }
            } else if (stripped.size() >= 8 && stripped.substr(0, 8) == "RCPT TO:") {
                if (!state->received_hello) {
                    write_response(connfd, bad_sequence_message);
                    return 0;
                }

                std::string recipient = get_email(stripped, 8);
                if (recipient == "") {
                    write_response(connfd, unknown_params_message);
                    return 0;
                }

                int pos = recipient.find("@");
                std::string user = recipient.substr(0, pos);
                std::string domain = recipient.substr(pos + 1);

                if (domain == "localhost") {
                    if (user_file_index.find(user) != user_file_index.end()) {
                        state->recipients.push_back(recipient);
                        write_response(connfd, ok_message);
                    } else {
                        write_response(connfd, mailbox_not_found);
                    }
                } else {
                    // Write to mqueue
                    state->recipients.push_back(recipient);
                    write_response(connfd, ok_message);
                }
            } else if (stripped == "DATA") {
                if (!state->received_hello || state->sender.size() == 0 ||
                    state->recipients.size() == 0) {
                    write_response(connfd, bad_sequence_message);
                } else {
                    state->reading_email_data = 1;
                    write_response(connfd, ready_to_receive);
                }
            } else if (stripped == "QUIT") {
                write_response(connfd, shutting_down_message);
                return 1;
            } else if (stripped == "RSET") {
                if (!state->received_hello) {
                    write_response(connfd, bad_sequence_message);
                } else {
                    reset_state(state);
                    write_response(connfd, ok_message);
                }
            } else if (stripped == "NOOP") {
                if (!state->received_hello) {
                    write_response(connfd, bad_sequence_message);
                } else {
                    write_response(connfd, ok_message);
                }
            } else {
                write_response(connfd, unknown_cmd_message);
            }
        }
        return 0;
    }
};

extern SMTPServer *smtp_server;

void *worker_func_wrapper(void *arg) {
    smtp_server->worker_func(arg);
    return NULL;
}

void int_handler_wrapper(int sig) { smtp_server->int_handler(sig); }

#endif
