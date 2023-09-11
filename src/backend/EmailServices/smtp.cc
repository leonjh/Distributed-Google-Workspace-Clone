#include "parse_input.h"
#include "smtpserver.hpp"
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

int portno = 2500;
bool debug_output = false;
std::string directory = "";

SMTPServer *server;

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    SMTPServer ss;
    server = &ss;

    signal(SIGINT, int_handler_wrapper);

    server->start();
    server->server_loop();

    return 0;
}
