#include <fstream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>

#include "../lib/constants.h"
#include "BaseServer.hpp"
#include "definitions.hpp"
#include "utils.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/storageserver.grpc.pb.h"
#else
#include "storageserver.grpc.pb.h"
#endif

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
  std::unique_ptr<StorageServer::Stub> stub_;

public:
  MainServer(int port, int v_flag, std::string static_dir_path, std::shared_ptr<Channel> channel)
      : BaseServer{port, v_flag}, static_dir_path{static_dir_path}, stub_(StorageServer::NewStub(channel))
  {
  }

  bool handle_request(Connection *connection, Request *request)
  {
    log(connection->comm_fd, "Incoming Request: " + request->message, v_flag);

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

      if (request->http_message->type == "GET")
      {
        std::string response;
        if (request->http_message->path == "/")
        {
          // initial http, no cookies found - havent log in
          if (request->http_message->header.find("Cookie") == request->http_message->header.end())
          {
            // no cookie, redirect to login
            std::string uuid = get_uuid();
            std::cout << "uuid " << uuid << std::endl;
            send_put_request("tmp", "col", uuid);

            std::string not_found_path = static_dir_path + "/index.html";
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
            std::string send_content = std::to_string(content.size()) + "\r\n\r\n" + content;
            response = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nSet-Cookie: sid=" + uuid +
                       "\r\nContent-length: " + send_content; // This includes the cookie - might not be needed
            std::cout << "This is the response " << response << std::endl;
            // response = "HTTP/1.1 307 Temporary redirect\r\nLocation: http://127.0.0.1:" + std::to_string(port) +
            //            "/login.html\r\nContent-type: text/html\r\nSet-Cookie: sid=" + uuid +
            //            "\r\nContent-length: 0\r\n\r\n";
          }
          else 
          { // TODO: check if logged in
            // home page
            response = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: "
                       "47\r\n\r\n<html><body><h1>Hello world!"
                       "</h1><button>Here </button></body></html>";
          }
        } else if (request->http_message->type == "OPTIONS")
        { // cors request
          std::cout << request->http_message->header["Origin"] << std::endl;
          response = "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: " + request->http_message->header["Origin"] 
          + "\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: X-PINGOTHER, Content-Type\r\nVary: Accept-Encoding, Origin\r\nKeep-Alive: timeout=2, max=100\r\nConnection: Keep-Alive\r\n\r\n";
        }
        else
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
            response = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: ";
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

        if (!do_write(connection->comm_fd, response, response.length()))
        {
          fprintf(stderr, "Write error");
          exit(EXIT_SUCCESS);
        }
        log(connection->comm_fd, "Response: " + response, v_flag);
      }
      else if (request->http_message->type == "POST")
      {
        std::string response;

        if (request->http_message->path == "/login.html")
        {
          std::cout << "From login.html" << std::endl;
        }
      }
    }

    return false; // true if server should stop reading
  }

private:
  std::string get_uuid()
  {
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

    std::string res;
    for (int i = 0; i < 16; i++)
    {
      if (dash[i])
        res += "-";
      res += v[dist(rng)];
      res += v[dist(rng)];
    }
    return res;
  }

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  // True = success, false = failure somewhere - will be logged.
  bool send_put_request(const std::string &row, const std::string &col, const std::string &value)
  {
    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1(value);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StoragePut(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return true;
    }
    else
    {
      log_message("%d: %s", status.error_code(), (char *)std::string(status.error_message()).c_str());
      return false;
    }
  }

  std::string send_get_request(const std::string &row, const std::string &col)
  {
    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StorageGet(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return response.value();
    }
    else
    {
      log_message("%d: %s", status.error_code(), (char *)std::string(status.error_message()).c_str());
      return "RPC failed";
    }
  }

  // Mock user registration function
  std::string register_new_user(const std::string &username, const std::string &password)
  {
    if (check_user_exists(username))
    {
      return "Username taken!";
    }

    std::string row = "user_" + username;
    std::string col = "password";
    if (send_put_request(row, col, password))
    {
      return "User created";
    }
    else
    {
      log_message("Failed to create user, check server logs");
      return "Failed to create user";
    }
  }

  // Mock user-exists function
  bool check_user_exists(const std::string &username)
  {
    std::string row = "user_" + username;
    std::string col = "password";
    std::string value = send_get_request(row, col);

    if (value == "RPC failed")
    {
      return false;
    }

    return true;
  }

  // Mock user/password confirmation function
  bool check_user_login(const std::string &username, const std::string &password)
  {
    std::string row = "user_" + username;
    std::string col = "password";
    std::string value = send_get_request(row, col);

    if (value == "RPC failed")
    {
      return false;
    }

    return value == password;
  }
};

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

  MainServer ms{parsed_info.first, parsed_info.second, static_dir_path,
                grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials())};
  server = &ms;
  server->start();

  return 0;
}
