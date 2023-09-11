#ifndef PARSE_INPUT_H
#define PARSE_INPUT_H

#include <string>

extern int portno;
extern bool debug_output;
extern std::string directory;

void parse_args(int argc, char *argv[]);

void raise_error(const std::string &msg);

void print_info(const std::string &msg);

void debug_new_conn(int fd);

void debug_received(int fd, const std::string &msg);

void debug_sent(int fd, const std::string &msg);

void debug_closed_conn(int fd);

#endif
