#ifndef ACCOUNT_SERVICE_HPP
#define ACCOUNT_SERVICE_HPP

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "../lib/constants.h"
#include "../lib/tokenizer.h"
#include "backend_interface.hpp"
#include "email.hpp"
#include "json.hpp"
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

using json = nlohmann::json;

class AccountService
{
public:
  AccountService() {}

  std::string handle_request(Request *request)
  {
    std::cout << "in1" << std::endl;
    std::pair<bool, json> response;

    json request_body = json::parse(request->http_message->body);
    std::cout << "It made it past the first parse" << std::endl;
    if (request->http_message->path == "/api/account/login") // Login
    {
      std::cout << "in2" << std::endl;
      response = login(request_body);
    }
    else if (request->http_message->path == "/api/account/signup")
    {
      response = signup(request_body);
    }

    int status = response.first ? 200 : 400;
    std::string additional_headers;
    if (response.first)
    {
      std::string username = request_body["username"];
      std::string sid = get_unique_id();
      additional_headers = "\r\nSet-Cookie: sid=" + sid +
                           ";Path=/; Secure; SameSite=None;Max-Age=99999999\r\nSet-Cookie: username=" + username +
                           ";Path=/; Secure; SameSite=None;Max-Age=99999999";

      BackendInterface::send_put_request(username + "_acc", "sid", sid);
    }
    return get_api_response(request, response.second, status, additional_headers);
  }

private:
  // additional headers
  std::pair<bool, json> login(json request_body)
  {
    json response_body;

    if (!check_user_exists(request_body["username"])) // user doesn't exist
    {
      response_body["message"] = "User doesn't exist";
      return {false, response_body};
    }

    if (!check_user_login(request_body["username"], request_body["password"])) // incorrect password
    {
      response_body["message"] = "Incorrect password";
      return {false, response_body};
    }

    // logged in
    response_body["message"] = "Authenticated";
    std::string username = request_body["username"];

    return {true, response_body};
  }

  std::pair<bool, json> signup(json request_body)
  {
    json response_body;

    std::pair<bool, std::string> register_res = register_new_user(request_body["username"], request_body["password"]);
    response_body["message"] = register_res.second;
    if (!register_res.first)
    {
      return {false, response_body};
    }

    return {true, response_body};
  }

  // Mock user registration function
  std::pair<bool, std::string> register_new_user(const std::string &username, const std::string &password)
  {
    if (check_user_exists(username))
    {
      return {false, "Username taken!"};
    }

    std::string acc_row = username + "_acc";
    std::string pass_col = "password";
    std::string mailbox_row = username + "_mbox";
    std::string files_row = username + "_files";
    std::string received_col = "received_ids";
    std::string sent_col = "sent_ids";

    if (BackendInterface::send_put_request(acc_row, pass_col, password) &&
        BackendInterface::send_put_request(mailbox_row, received_col, "") &&
        BackendInterface::send_put_request(mailbox_row, sent_col, "") &&
        BackendInterface::send_put_request(files_row, "root:content", "[\"trash\", \"recents\"]") &&
        BackendInterface::send_put_request(files_row, "trash:content", "[]") &&
        BackendInterface::send_put_request(files_row, "recents:content", "[]") &&
        BackendInterface::send_put_request(files_row, "trash:name", "Trash") &&
        BackendInterface::send_put_request(files_row, "recents:name", "Recents") &&
        BackendInterface::send_put_request(files_row, "trash:metadata", "{}") &&
        BackendInterface::send_put_request(files_row, "recents:metadata", "{}"))
    {
      return {true, "User created"};
    }
    else
    {
      return {false, "Failed to create user"};
    }
  }

  // Mock user-exists function
  bool check_user_exists(const std::string &username)
  {
    std::string row = username + "_acc";
    std::string col = "password";
    std::pair<bool, std::string> res = BackendInterface::send_get_request(row, col);
    std::string value = res.second;

    std::cout << "RPC value: " << value << std::endl;
    return res.first;
  }

  // Mock user/password confirmation function
  bool check_user_login(const std::string &username, const std::string &password)
  {
    std::string row = username + "_acc";
    std::string col = "password";
    std::pair<bool, std::string> res = BackendInterface::send_get_request(row, col);
    std::string value = res.second;

    if (!res.first)
    {
      return false;
    }

    return value == password;
  }
};

#endif