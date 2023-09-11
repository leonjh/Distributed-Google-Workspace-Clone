#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "BaseServer.hpp"
#include "utils.hpp"
#include "definitions.hpp"

// TODO: close frontend server connections

class FrontendLoadBalancer : public BaseServer
{
private:
  typedef struct server
  {
    std::string full_addr;
    int sock_fd;
  } Server;

  std::vector<Server *> frontend_servers;

  void init_servers(std::string config_file_path)
  {
    // read server infos from config file
    std::fstream server_config{config_file_path, std::ios::in};
    if (server_config.is_open())
    {
      std::string line;
      while (std::getline(server_config, line))
      {
        Server *frontend_server = new Server();
        frontend_server->full_addr = line;

        // create comm
        int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0)
        {
          perror("socket():");
          exit(EXIT_FAILURE);
        }
        frontend_server->sock_fd = sock_fd;

        std::pair<std::string, std::string> parsed_full_addr = parse_full_addr(frontend_server->full_addr);

        // create sockaddr for server to server comm
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(std::stoi(parsed_full_addr.second));
        inet_pton(AF_INET, parsed_full_addr.second.c_str(), &servaddr.sin_addr);
        int ret = connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0)
        {
          perror("connect():");
          exit(EXIT_FAILURE);
        }

        log(-1, "Server setup: found " + frontend_server->full_addr, v_flag);
        frontend_servers.push_back(frontend_server);
      }
    }
  }

public:
  FrontendLoadBalancer(int port, int v_flag, std::string config_file_path) : BaseServer{port, v_flag}
  {
    init_servers(config_file_path);
  }

  bool handle_request(Connection *connection, Request *request)
  {
    log(connection->comm_fd, "Incoming Request: " + request->message, v_flag);

    if (request->type == CLIENT_REQUEST)
    {
      int min_num_clients = INT_MAX;
      int min_server_index = -1;

      for (int i = 0; i < frontend_servers.size(); i++)
      {

        // create comm
        int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0)
        {
          perror("socket():");
          exit(EXIT_FAILURE);
        }

        std::pair<std::string, std::string> parsed_full_addr = parse_full_addr(frontend_servers[i]->full_addr);

        // create sockaddr for server to server comm
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(std::stoi(parsed_full_addr.second));
        inet_pton(AF_INET, parsed_full_addr.second.c_str(), &servaddr.sin_addr);
        int ret = connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0)
        {
          std::cout << "couldnt connect with " << i << std::endl;
          continue;
        }

        std::string lb_request = "!INFO\r\n";
        if (!do_write(sock_fd, lb_request, lb_request.length()))
        {
          fprintf(stderr, "Write error\n");
          exit(EXIT_FAILURE);
        }
        log(sock_fd, "Outgoing Request: " + lb_request, v_flag);

        std::string response = do_read(sock_fd);
        if (response.length() > 0)
        {
          int num_clients = atoi(response.substr(1, response.length() - 1).c_str());
          if (num_clients < min_num_clients)
          {
            min_num_clients = num_clients;
            min_server_index = i;
          }
        }
        else
        {
          std::cout << "Error: Read from " << i << "not found" << std::endl;
        }
        log(sock_fd,
            "Load Response: " + response.substr(0, response.length() - 2) + " from " + frontend_servers[i]->full_addr,
            v_flag);
      }

      // read response
      // int min_num_clients = INT_MAX;
      // int min_server_index = -1;
      // for (int i = 0; i < frontend_servers.size(); i++)
      // {
      //   std::string response = do_read(frontend_servers[i]->sock_fd);
      //   if (response.length() > 0)
      //   {
      //     int num_clients = atoi(response.substr(1, response.length() - 1).c_str());
      //     if (num_clients < min_num_clients)
      //     {
      //       min_num_clients = num_clients;
      //       min_server_index = i;
      //     }
      //   }
      //   else
      //   {
      //     std::cout << "Error: Read from " << i << "not found" << std::endl;
      //   }
      //   log(frontend_servers[i]->sock_fd,
      //       "Load Response: " + response.substr(0, response.length() - 2) + " from " + frontend_servers[i]->full_addr,
      //       v_flag);
      // }

      if (min_server_index == -1)
      {
        std::cout << "Error: No frontend server picked" << std::endl;
        return false;
      }

      std::string client_response = "HTTP/1.1 307 Temporary redirect\r\nLocation: http://" +
                                    frontend_servers[min_server_index]->full_addr +
                                    "/\r\nContent-type: text/html\r\nContent-length: 0\r\n\r\n";
      if (!do_write(connection->comm_fd, client_response, client_response.length()))
      {
        fprintf(stderr, "Write error");
      }
      log(connection->comm_fd, "Response: " + client_response, v_flag);

      connection->thread_status = THREAD_NEED_CLEANUP; // end client connection and cleanup
      return true;
    }

    return false;
  }
};

int main(int argc, char *argv[])
{
  std::pair<int, int> parsed_info = parse_command(argc, argv);

  if (optind >= argc)
  {
    fprintf(stderr, "Config file not specified\n");
    exit(EXIT_FAILURE);
  }

  FrontendLoadBalancer lb{parsed_info.first, parsed_info.second, argv[optind]};
  server = &lb;
  server->start();
}
