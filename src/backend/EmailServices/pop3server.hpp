#ifndef POP3_SERVER_HPP
#define POP3_SERVER_HPP

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
 * POP3 Server implementation
 */
class POP3Server : public BaseServer {
  private:
    const std::string greeting_message =
        "+OK POP3 server ready (Author: Joseph Zhang / jzhang25)\r\n";
    const std::string unknown_params_message = "-ERR Syntax error in parameters\r\n";
    const std::string bad_sequence_message = "-ERR Bad sequence of commands\r\n";
    const std::string quit_message_good =
        "+OK localhost POP3 server closing transmission channel\r\n";
    const std::string quit_message_bad =
        "-ERR localhost POP3 server closing transmission channel\r\n";
    const std::string update_failed_message = "-ERR Some deleted messages not removed\r\n";
    const std::string mailbox_not_found = "-ERR No such mailbox here\r\n";
    const std::string unsupported_cmd_message = "-ERR Not supported\r\n";
    const std::string invalid_password = "-ERR Invalid password\r\n";
    const std::string lock_mailbox_failed = "-ERR Unable to lock mailbox\r\n";
    const std::vector<std::string> supported_cmds = {"USER", "PASS", "QUIT", "STAT", "LIST",
                                                     "DELE", "NOOP", "RSET", "RETR", "UIDL"};

    struct message_state {
        long octets;       // email size in octets
        long offset;       // offset of email from beginning of mbox file
        long total_length; // total size of email including metadata
    };

    struct session_state {
        bool authenticated; // is user authenticated
        std::string user;   // username
        std::vector<message_state> mailbox_state;
        std::unordered_set<int> transactions; // message indices client wants to delete
        std::unique_lock<std::mutex> lock;    // thread-level lock on mbox file
        int fd;                               // file descriptor of mbox file
    };

    std::unordered_map<std::string, int> user_file_index;
    std::vector<std::string> files;
    std::vector<std::unique_ptr<std::mutex>> file_locks;

  public:
    /**
     * Initialize file locks
     */
    POP3Server() {
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
            write(i, quit_message_bad.c_str(), strlen(quit_message_bad.c_str()));
            if (debug_output) {
                debug_sent(i, quit_message_bad);
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

        struct session_state tmp_state = {0, "", {}, {}, {}, -1};
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
                        if (state->fd > -1) {
                            if (flock(state->fd, LOCK_UN) < 0) {
                                perror("flock(): ");
                            }
                            close(state->fd);
                        }
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
     * Reset state (clear transactions)
     */
    void reset_state(struct session_state *state) { state->transactions = {}; }

    /**
     * Get mailbox state by parsing each message from user mailbox and saving metadata like offset
     * and size
     */
    void get_mailbox_state(struct session_state *state) {
        FILE *file = fdopen(state->fd, "r");
        char *line = NULL;
        ssize_t read;
        size_t len;
        bool begin = 1;
        struct message_state message = {0, 0, 0};
        int offset = 0;
        while ((read = getline(&line, &len, file)) != -1) {
            if (read > 6 && line[0] == 'F' && line[1] == 'r' && line[2] == 'o' && line[3] == 'm' &&
                line[4] == ' ' && line[5] == '<') {
                if (offset) // handle old message
                {
                    state->mailbox_state.push_back(message);
                    message = {0, 0, 0};
                }
                message.offset = offset;
                message.total_length += read;
            } else {
                message.total_length += read;
                message.octets += read;
            }
            offset += read;
        }
        if (offset) {
            state->mailbox_state.push_back(message);
        }
        free(line);
    }

    /**
     * Create ok message describing maildrop info
     */
    std::string mailbox_info_ok_message(struct session_state *state) {
        int total_octets = 0;
        for (auto &message : state->mailbox_state) {
            total_octets += message.octets;
        }
        std::string message_count =
            "+OK maildrop has " + std::to_string(state->mailbox_state.size()) + " messages";
        std::string octet_count = "(" + std::to_string(total_octets) + " octets)";
        return message_count + " " + octet_count + "\r\n";
    }

    /**
     * Helper function to split string by spaces and convert to vector
     */
    std::vector<std::string> split(const std::string &s) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, ' ')) {
            if (item == "") {
                continue;
            }
            result.push_back(item);
        }
        return result;
    }

    /**
     * Read a message from user mailbox given offset and length
     */
    std::string read_message(int fd, struct message_state *ms, bool include_metadata) {
        if (!include_metadata) {
            char buf[ms->octets + 1];
            if (pread(fd, buf, ms->octets, ms->offset + (ms->total_length - ms->octets)) < 0) {
                perror("pread(): ");
            }
            buf[ms->octets] = '\0';
            return std::string(buf);
        } else {
            char buf[ms->total_length + 1];
            if (pread(fd, buf, ms->total_length, ms->offset) < 0) {
                perror("pread(): ");
            }
            buf[ms->total_length] = '\0';
            return std::string(buf);
        }
    }

    /**
     * Compute digest of message and convert to string
     */
    std::string compute_digest(const char *data, int dataLengthBytes) {
        /* The digest will be written to digestBuffer, which must be at least MD5_DIGEST_LENGTH
         * bytes long */
        unsigned char digestBuffer[MD5_DIGEST_LENGTH + 1];
        MD5_CTX c;
        MD5_Init(&c);
        MD5_Update(&c, data, dataLengthBytes);
        MD5_Final(digestBuffer, &c);
        std::string result;
        result.reserve(32);
        for (std::size_t i = 0; i < 16; ++i) {
            result += "0123456789ABCDEF"[digestBuffer[i] / 16];
            result += "0123456789ABCDEF"[digestBuffer[i] % 16];
        }
        return result;
    }

    /**
     * Update mailbox after QUIT command
     */
    bool update(struct session_state *state) {
        if (state->transactions.empty()) {
            return 1;
        }

        FILE *file = fdopen(state->fd, "r+");
        if (file == NULL) {
            perror("fdopen(): ");
            return 0;
        }
        rewind(file); // set file pointer to offset 0
        int total_bytes = 0;
        for (int i = 0; i < state->mailbox_state.size(); ++i) {
            if (state->transactions.find(i) != state->transactions.end()) {
                continue;
            }
            std::string message = read_message(state->fd, &state->mailbox_state[i], true);
            total_bytes += sizeof(char) * message.size();
            if (fprintf(file, message.c_str(), sizeof(char) * message.size()) < 0) {
                perror("fprintf(): ");
                return 0;
            }
        }
        rewind(file);
        if (ftruncate(state->fd, total_bytes) < 0) // truncate file to new size
        {
            perror("ftruncate(): ");
        }
        return 1;
    }

    /**
     * Process command received from clientpos
     */
    bool process(int connfd, const std::string &cmd, struct session_state *state) {
        if (debug_output) {
            debug_received(connfd, cmd);
        }

        std::string stripped = cmd.substr(0, cmd.size() - 2);
        for (int i = 0; i < stripped.size() && stripped[i] != ' '; ++i) {
            if (isalpha(stripped[i])) {
                stripped[i] = toupper(stripped[i]);
            }
        }

        if (stripped.empty()) {
            return 0;
        }

        std::vector<std::string> toks = split(stripped);
        if (toks[0] == "USER") {
            if (toks.size() != 2) {
                write_response(connfd, unknown_params_message);
            } else if (state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                state->user.clear();
                if (user_file_index.find(toks[1]) != user_file_index.end()) {
                    state->user = toks[1];
                    write_response(connfd, "+OK found user " + state->user + "\r\n");
                } else {
                    write_response(connfd, mailbox_not_found);
                }
            }
        } else if (toks[0] == "PASS") {
            if (toks.size() != 2) {
                write_response(connfd, unknown_params_message);
            } else if (state->authenticated || state->user.empty()) {
                write_response(connfd, bad_sequence_message);
            } else {
                if (stripped.substr(5) != "cis505") {
                    write_response(connfd, invalid_password);
                } else {
                    int index = user_file_index[state->user];
                    state->fd = open(files[index].c_str(), O_RDWR);
                    state->lock =
                        std::unique_lock<std::mutex>(*file_locks[index], std::try_to_lock);
                    if (flock(state->fd, LOCK_EX | LOCK_NB) < 0 ||
                        !state->lock.owns_lock()) // check if locks were acquired
                    {
                        write_response(connfd, lock_mailbox_failed);
                        state->fd = -1;
                        state->lock = {};
                    } else {
                        state->authenticated = 1;
                        get_mailbox_state(state);
                        write_response(connfd, mailbox_info_ok_message(state));
                    }
                }
            }
        } else if (stripped == "STAT") {
            if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                int nn = state->mailbox_state.size() - state->transactions.size();
                int mm = 0;
                for (int i = 0; i < state->mailbox_state.size(); ++i) {
                    if (state->transactions.find(i) != state->transactions.end()) {
                        continue;
                    }
                    mm += state->mailbox_state[i].octets;
                }
                write_response(connfd,
                               "+OK " + std::to_string(nn) + " " + std::to_string(mm) + "\r\n");
            }
        } else if (toks[0] == "LIST") {
            if (toks.size() > 2) {
                write_response(connfd, unknown_params_message);
            } else if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                int nn = state->mailbox_state.size() - state->transactions.size();
                if (toks.size() == 1) {
                    int total_octets = 0;
                    std::string lines = "";
                    for (int i = 0; i < state->mailbox_state.size(); ++i) {
                        if (state->transactions.find(i) != state->transactions.end()) {
                            continue;
                        }
                        lines += std::to_string(i + 1) + " " +
                                 std::to_string(state->mailbox_state[i].octets) + "\r\n";
                        total_octets += state->mailbox_state[i].octets;
                    }
                    lines += ".\r\n";
                    std::string header = "+OK " + std::to_string(nn) + " messages (" +
                                         std::to_string(total_octets) + " octets)\r\n";
                    write_response(connfd, header + lines);
                } else {
                    try {
                        int index = std::stoi(toks[1]) - 1;
                        if (index >= state->mailbox_state.size() || index < 0) {
                            write_response(connfd, "-ERR no such message, only " +
                                                       std::to_string(nn) +
                                                       " messages in maildrop\r\n");
                        } else if (state->transactions.find(index) != state->transactions.end()) {
                            write_response(connfd, "-ERR message " + toks[1] + " was deleted\r\n");
                        } else {
                            write_response(connfd,
                                           "+OK " + toks[1] + " " +
                                               std::to_string(state->mailbox_state[index].octets) +
                                               "\r\n");
                        }
                    } catch (const std::exception &e) {
                        write_response(connfd, unknown_params_message);
                    }
                }
            }
        } else if (toks[0] == "DELE") {
            if (toks.size() != 2) {
                write_response(connfd, unknown_params_message);
            } else if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                try {
                    int nn = state->mailbox_state.size();
                    int index = std::stoi(toks[1]) - 1;
                    if (index >= nn || index < 0) {
                        write_response(connfd, "-ERR no such message\r\n");
                    } else if (state->transactions.find(index) != state->transactions.end()) {
                        write_response(connfd, "-ERR message " + toks[1] + " was deleted\r\n");
                    } else {
                        state->transactions.insert(index);
                        write_response(connfd, "+OK message " + toks[1] + " deleted\r\n");
                    }
                } catch (const std::exception &e) {
                    write_response(connfd, unknown_params_message);
                }
            }
        } else if (toks[0] == "RETR") {
            if (toks.size() != 2) {
                write_response(connfd, unknown_params_message);
            } else if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                try {
                    int nn = state->mailbox_state.size();
                    int index = std::stoi(toks[1]) - 1;
                    if (index >= nn || index < 0) {
                        write_response(connfd, "-ERR no such message\r\n");
                    } else if (state->transactions.find(index) != state->transactions.end()) {
                        write_response(connfd, "-ERR message " + toks[1] + " was deleted\r\n");
                    } else {
                        struct message_state *ms = &state->mailbox_state[index];
                        std::string header = "+OK " + std::to_string(ms->octets) + " octets\r\n";
                        std::string body = read_message(state->fd, ms, false) + ".\r\n";
                        write_response(connfd, header + body);
                    }
                } catch (const std::exception &e) {
                    write_response(connfd, unknown_params_message);
                }
            }
        } else if (toks[0] == "UIDL") {
            if (toks.size() > 2) {
                write_response(connfd, unknown_params_message);
            } else if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                if (toks.size() == 1) {
                    std::string response = "+OK\r\n";
                    for (int i = 0; i < state->mailbox_state.size(); ++i) {
                        if (state->transactions.find(i) != state->transactions.end()) {
                            continue;
                        }
                        std::string message =
                            read_message(state->fd, &state->mailbox_state[i], true);
                        response += std::to_string(i + 1) + " " +
                                    compute_digest(message.c_str(), message.size()) + "\r\n";
                    }
                    response += ".\r\n";
                    write_response(connfd, response);
                } else {
                    try {
                        int nn = state->mailbox_state.size();
                        int index = std::stoi(toks[1]) - 1;
                        if (index >= nn || index < 0) {
                            write_response(connfd, "-ERR no such message, only " +
                                                       std::to_string(nn) +
                                                       " messages in maildrop\r\n");
                        } else if (state->transactions.find(index) != state->transactions.end()) {
                            write_response(connfd, "-ERR message " + toks[1] + " was deleted\r\n");
                        } else {
                            std::string message =
                                read_message(state->fd, &state->mailbox_state[index], true);
                            write_response(connfd,
                                           "+OK " + toks[1] + " " +
                                               compute_digest(message.c_str(), message.size()) +
                                               "\r\n");
                        }
                    } catch (const std::exception &e) {
                        write_response(connfd, unknown_params_message);
                    }
                }
            }
        } else if (stripped == "QUIT") {
            if (!state->authenticated) {
                write_response(connfd, quit_message_good);
            } else {
                if (update(state)) {
                    write_response(connfd, quit_message_good);
                } else {
                    write_response(connfd, update_failed_message);
                }
            }
            return 1;
        } else if (stripped == "RSET") {
            if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                reset_state(state);
                write_response(connfd, mailbox_info_ok_message(state));
            }
        } else if (stripped == "NOOP") {
            if (!state->authenticated) {
                write_response(connfd, bad_sequence_message);
            } else {
                write_response(connfd, "+OK\r\n");
            }
        } else {
            write_response(connfd, unsupported_cmd_message);
        }
        return 0;
    }
};

extern POP3Server *server;

void *worker_func_wrapper(void *arg) {
    server->worker_func(arg);
    return NULL;
}

void int_handler_wrapper(int sig) { server->int_handler(sig); }

#endif
