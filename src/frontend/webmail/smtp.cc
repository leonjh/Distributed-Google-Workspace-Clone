#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "../backend_interface.hpp"
#include "parse_input.hpp"
#include "smtpserver.hpp"

int portno = 2500;
bool debug_output = false;
std::string directory = "";

SMTPServer *smtp_server;
std::unordered_map<std::string, std::vector<std::string>> BackendInterface::address_cache = {};

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    SMTPServer ss;
    smtp_server = &ss;

    signal(SIGINT, int_handler_wrapper);

    smtp_server->start();
    smtp_server->server_loop();

    return 0;
}