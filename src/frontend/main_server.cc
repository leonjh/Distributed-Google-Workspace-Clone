#include <fstream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>

#include "../lib/constants.h"
#include "BaseServer.hpp"
#include "account_service.hpp"
#include "admin_service.hpp"
#include "backend_interface.hpp"
#include "definitions.hpp"
#include "filesystem_service.hpp"
#include "json.hpp"
#include "utils.hpp"
#include "webmail_service.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/storageserver.grpc.pb.h"
#else
#include "storageserver.grpc.pb.h"
#endif

#define MAX_LOG_LENGTH 2000

using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class MainServer : public BaseServer
{
private:
  typedef struct client
  {
    int comm_fd;
  } Client;

  std::unordered_map<int, Client *> active_clients;
  std::string static_dir_path;
  std::unordered_set<std::string> valid_pages_url{"/", "/login", "/register", "/home"};

public:
  WebMailService webmail_server;
  AdminService admin_service;
  AccountService account_service;
  FileSystemService filesystem_service;

  MainServer(int port, int v_flag, std::string static_dir_path)
      : BaseServer{port, v_flag}, static_dir_path{static_dir_path}
  {
  }
  // reference: std::shared_ptr<Channel> channel

  bool handle_request(Connection *connection, Request *request)
  {
    log(connection->comm_fd, "Incoming Request: " + request->message.substr(0, MAX_LOG_LENGTH), v_flag);

    if (request->type == SERVER_REQUEST)
    {
      if (command_check(request->message.substr(0, request->message.length() - 2), "INFO"))
      {
        std::string response = "!" + std::to_string(active_clients.size()) + "\r\n";
        if (!do_write(connection->comm_fd, response, response.length()))
        {
          fprintf(stderr, "Write error");
          exit(EXIT_SUCCESS);
        }
      }
      else
      {
        log(connection->comm_fd, "Invalid command: " + request->message, v_flag);
      }
    }

    // add to client
    if (request->type == CLIENT_REQUEST)
    {
      if (active_clients.find(connection->comm_fd) == active_clients.end())
      {
        Client *client = new Client();
        client->comm_fd = connection->comm_fd;
        active_clients.insert({connection->comm_fd, client});
      }

      std::string response;
      if (request->http_message->type == "GET") // mostly frontend serving
      {
        std::string path = request->http_message->path;
        if (path.find("/api/webmail") != string::npos)
        {
          response = webmail_server.handle_request(request);
        }
        else if (path.find("/api/filesystem") != string::npos)
        {
          response = filesystem_service.handle_request(request);
        }
        else if (valid_pages_url.count(path)) // serve static dependencies
        {
          unordered_map<std::string, std::string> cookies;
          if (request->http_message->header.find("Cookie") != request->http_message->header.end())
          {
            std::string cookies_value = request->http_message->header["Cookie"];
            cookies = parse_cookies(cookies_value);
          }

          if (cookies.find("sid") == cookies.end() &&
              (request->http_message->path != "/login" && request->http_message->path != "/register"))
          {
            response = "HTTP/1.1 307 Temporary redirect\r\nLocation: http://localhost:" + std::to_string(port) +
                       "/login\r\nContent-type: text/html\r\nContent-length: 0\r\n\r\n";
          }
          else if (cookies.find("sid") != cookies.end() &&
                   (request->http_message->path == "/login" || request->http_message->path == "/register"))
          {
            response = "HTTP/1.1 307 Temporary redirect\r\nLocation: http://localhost:" + std::to_string(port) +
                       "/home\r\nContent-type: text/html\r\nContent-length: 0\r\n\r\n";
          }
          else
          {
            std::string index_file_path = static_dir_path + "/index.html";
            std::string content;
            std::fstream index_file{index_file_path, std::ios::in};
            if (index_file.is_open())
            {
              std::string line;
              while (std::getline(index_file, line))
              {
                content += line;
              }
            }
            std::string send_content = std::to_string(content.size()) + "\r\n\r\n" + content;
            response = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: " + send_content;
          }
        }
        else // serve index file
        {
          std::string static_file_path = static_dir_path + request->http_message->path;
          struct stat sb;
          if (stat(static_file_path.c_str(), &sb) != 0)
          {
            response = "HTTP/1.1 404 Not Found\r\nContent-type: text/html\r\nContent-length: ";
            std::string not_found_path = static_dir_path + "/404.html";
            std::string content;
            std::fstream not_found_file{not_found_path, std::ios::in};
            if (not_found_file.is_open())
            {
              std::string line;
              while (std::getline(not_found_file, line))
              {
                content += line;
              }
            }
            response += std::to_string(content.size()) + "\r\n\r\n" + content;
          }
          else
          {
            std::string content_type = (request->http_message->path.find(".png") == string::npos) ? "text/html" : "image/png";
            std::string content_disp = (request->http_message->path.find(".png") == string::npos) ? "" : "Content-Disposition: attachment;filename=\"email_icon.png\"\r\n";
            response = "HTTP/1.1 200 OK\r\nContent-type: " + content_type + "\r\n" + content_disp + "Content-length: ";
            std::string content;
            std::fstream static_file{static_file_path, std::ios::in};
            if (static_file.is_open())
            {
              std::string line;
              while (std::getline(static_file, line))
              {
                content += line;
              }
            }
            response += std::to_string(content.size()) + "\r\n\r\n" + content;
          }
        }
      }
      else if (request->http_message->type == "OPTIONS")
      { // CORS request
        response =
            "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: " + request->http_message->header["Origin"] +
            "\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: X-PINGOTHER, "
            "Content-Type, Authorization\r\nAccess-Control-Allow-Credentials: true\r\nVary: Accept-Encoding, "
            "Origin\r\nKeep-Alive: timeout=2, max=100\r\nConnection: "
            "Keep-Alive\r\n\r\n";
      }
      else if (request->http_message->type == "POST")
      {
        if (request->http_message->path.find("/api/account") != string::npos) 
        {
          response = account_service.handle_request(request);
        }
        else if (request->http_message->path.find("/api/webmail") != string::npos)
        {
          response = webmail_server.handle_request(request);
        }
        else if (request->http_message->path.find("/api/filesystem") != string::npos)
        {
          response = filesystem_service.handle_request(request);
        }
        else if (request->http_message->path.find("/api/admin") != string::npos)
        {
          response = admin_service.handle_request(request);
        }
      }

      if (!do_write(connection->comm_fd, response, response.length()))
      {
        fprintf(stderr, "Write error");
        exit(EXIT_SUCCESS);
      }
      log(connection->comm_fd, "Response: " + response.substr(0, MAX_LOG_LENGTH), v_flag);
    }

    return false; // true if server should stop reading
  }
};

std::unordered_map<std::string, std::vector<std::string>> BackendInterface::address_cache = {};

int main(int argc, char *argv[])
{
  std::pair<int, int> parsed_info = parse_command(argc, argv);

  if (optind >= argc)
  {
    fprintf(stderr, "Static files directory not specified\n");
    exit(EXIT_FAILURE);
  }

  std::string static_dir_path = argv[optind];
  struct stat sb;
  if (stat(static_dir_path.c_str(), &sb) != 0)
  {
    fprintf(stderr, "Path not found.\n");
    exit(EXIT_FAILURE);
  }

  MainServer ms{parsed_info.first, parsed_info.second, static_dir_path};
  server = &ms;
  ms.admin_service.set_server_port(parsed_info.first);
  ms.admin_service.test_admin_op();
  server->start();

  return 0;
}
