#include "parse_input.h"
#include "pop3server.hpp"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/md5.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

int portno = 11000;
bool debug_output = false;
std::string directory = "";

POP3Server *server;

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    POP3Server ps;
    server = &ps;

    signal(SIGINT, int_handler_wrapper);

    server->start();
    server->server_loop();

    return 0;
}
