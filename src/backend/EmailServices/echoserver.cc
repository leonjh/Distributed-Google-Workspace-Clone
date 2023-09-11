#include "echoserver.hpp"
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

int portno = 10000;
bool debug_output = false;
std::string directory = "";

EchoServer *server;

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    EchoServer es;
    server = &es;

    signal(SIGINT, int_handler_wrapper);

    server->start();
    server->server_loop();

    return 0;
}
