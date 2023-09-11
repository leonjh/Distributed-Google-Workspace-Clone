#ifndef ADMIN_SERVICE_HPP
#define ADMIN_SERVICE_HPP

#include <fstream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../lib/constants.h"
#include "../lib/tokenizer.h"
#include "backend_interface.hpp"
#include "definitions.hpp"
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

using backend::KillStorageServerRequest;
using backend::MasterEmpty;
using backend::MasterGetKVStoreStateResponse;
using backend::MasterServer;
using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using json = nlohmann::json;

class AdminService
{
private:
  int server_port;

public:
  AdminService() {}

  std::string handle_request(Request *request)
  {
    std::cout << "enter admin handler" << std::endl;
    std::pair<bool, json> response;
    json request_body = json::parse(request->http_message->body);

    std::unordered_map<std::string, std::string> cookies;

    json no_cookie_json;
    no_cookie_json["message"] = "Not authenticated";
    response.second = no_cookie_json;
    response.first = false;

    if (request->http_message->header.find("Cookie") != request->http_message->header.end())
    {
      std::cout << request->http_message->header["Cookie"] << std::endl;
      cookies = parse_cookies(request->http_message->header["Cookie"]);
      if (cookies.find("username") != cookies.end())
      {
        if (request->http_message->path == "/api/admin/toggle_server")
        {
          response = toggle_server(request_body);
          // test_admin_op();
        }
        else if (request->http_message->path == "/api/admin/get_http_server_status")
        {
          response = check_frontend_statuses();
        }
        else if (request->http_message->path == "/api/admin/get_server_status")
        {
          response = get_all_statuses();
        }
        else if (request->http_message->path == "/api/admin/retrieve_all_rows")
        {
          response = get_all_rows();
        }
        else if (request->http_message->path == "/api/admin/retrieve_row_data")
        {
          response = get_row_data(request_body);
        }
      }
    }

    int status = response.first ? 200 : 400;
    return get_api_response(request, response.second, status, "");
  }

  /**
   * @brief Set the main_server port value
   *
   * @param port - The port the main_server is running on
   */
  void set_server_port(int port) { server_port = port; }

  /**
   * @brief Get the main_server port value
   *
   * @return int - The port
   */
  int get_server_port() { return server_port; }

  void test_admin_op()
  {
    json request_body;
    std::string ip_get_test = get_storage_ip(1);
    std::cout << ip_get_test << std::endl;
    ip_get_test = get_storage_ip(4);
    std::cout << ip_get_test << std::endl;
    ip_get_test = get_storage_ip(11);
    std::cout << ip_get_test << std::endl;

    std::pair<bool, json> outty = BackendInterface::send_status_request();
    if (outty.first)
    {
      std::cout << outty.second.dump(4) << std::endl;
    }

    std::pair<bool, json> kv_json = BackendInterface::get_kv_state();
    std::cout << "This is the kv_json" << std::endl;
    // std::cout << kv_json.second.dump(4) << std::endl;

    std::pair<bool, json> frontend_statuses = check_frontend_statuses();
    if (frontend_statuses.first)
    {
      std::cout << frontend_statuses.second.dump(4) << std::endl;
    }
  }

  /**
   * @brief Checks if each frontend server is currently running or not, and returns a json with
   * corresponding information
   *
   * @return std::pair<bool, json>  - The response json and true/false if it worked
   */
  std::pair<bool, json> check_frontend_statuses()
  {
    json response_body;

    // Open the frontend servers config file so we can parse it, throw an error if not found
    std::string frontend_config_file = "./config/frontend";
    std::fstream frontend_file{frontend_config_file, std::ios::in};
    if (!frontend_file.is_open())
    {
      std::cout << "Error opening backend file config" << std::endl;
      response_body["message"] = "frontend config file no longer exists.";
      return {false, response_body};
    }

    // The server index we're testing
    int index = 0;

    std::string line;
    while (std::getline(frontend_file, line))
    {
      // Create a socket to check if each frontend server is running
      int sock = socket(AF_INET, SOCK_STREAM, 0);
      struct sockaddr_in addr;

      // Get the IP and Port of the currently line
      std::pair<std::string, std::string> parsed_full_addr = parse_full_addr(line);

      // Prepare the servaddr_in for connecting
      bzero(&addr, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(std::stoi(parsed_full_addr.second));
      inet_pton(AF_INET, parsed_full_addr.second.c_str(), &addr.sin_addr);

      // If we can connect add true to the response
      if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0)
      {
        response_body[to_string(++index)] = true;
      }
      else
      {
        if (std::stoi(parsed_full_addr.second) == server_port)
        {
          response_body[to_string(++index)] = true;
        }
        else
        {
          response_body[to_string(++index)] = false;
        }
      }

      close(sock);
    }

    // If index is 0 (i.e. we never read a single line) return false, must be an error
    // Shouldn't happen, but just incase.
    if (index == 0)
    {
      response_body["message"] = "Error with frontend server config file.";
      return {false, response_body};
    }

    return {true, response_body};
  }

  /**
   * @brief Get the all rows object
   *
   * @return std::pair<bool, json>
   */
  std::pair<bool, json> get_all_statuses()
  {
    json response_json;

    // Try to retrieve the rows & all column data for the storage servers
    std::pair<bool, json> status_req = BackendInterface::send_status_request();

    // If it was sucessful return the retrieved data
    if (status_req.first)
    {
      response_json = status_req.second;
      return {true, response_json};
    }

    response_json["message"] = "Could not retrieve server statuses";
    return {false, response_json};
  }

  /**
   * @brief Get the all rows object
   *
   * @return std::pair<bool, json>
   */
  std::pair<bool, json> get_all_rows()
  {
    json response_json;

    // Try to retrieve the rows & all column data for the storage servers
    std::pair<bool, json> row_cols = BackendInterface::get_kv_state();

    // If it was sucessful return the retrieved data
    if (row_cols.first)
    {
      response_json = row_cols.second;
      return {true, response_json};
    }

    response_json["message"] = "Could not retrieve row & column data";
    return {false, response_json};
  }

  /**
   * @brief Get the storage ip based off provided index
   *
   * @param index - The number of the storage server
   * @return std::string - The ip address of the storage server, NO_FILE if file doesnt exist, or
   * OUT_OF_BOUNDS if the range isn't valid
   */
  std::string get_storage_ip(int index)
  {
    // Open the file containing the backend server ip config
    std::string backend_config_file = "./config/backend";
    std::fstream backend_file{backend_config_file, std::ios::in};
    if (!backend_file.is_open())
    {
      std::cout << "Error opening backend file config" << std::endl;
      return "NO_FILE";
    }

    // Parse the file line by line until we reache the index we desire
    int cur_index = 0;
    std::string line;
    while (std::getline(backend_file, line))
    {
      if (++cur_index == index)
      {
        // Return the ip:port of the server once the index is reached
        return line;
      }
    }

    backend_file.close();
    return "OUT_OF_BOUNDS";
  }

  /**
   * @brief Processes a toggle server request, attempting to change the server as specified
   *
   * @param request_body - The json representation of the post request
   * @return std::pair<bool, json> - Whether it was sucessful or not for the HTML message
   */
  std::pair<bool, json> toggle_server(json request_body)
  {
    json response_body;

    // Confirm that the request has the server & a toggle change
    if (!request_body.contains("server") || !request_body.contains("toggle"))
    {
      response_body["message"] = "Request was missing a specified server and/or specified toggle.";
      return {false, response_body};
    }

    // Get the textual ip & port of the storage server we're requesting to shut down.
    std::string toggle_server_ip = get_storage_ip(request_body["server"]);

    std::string action = request_body["toggle"];
    if (action.compare("off") == 0)
    {
      // Invoke wakeup command
      bool kill_desired_server = BackendInterface::send_kill_request(toggle_server_ip);

      // Set the message to succesfully woken up
      if (kill_desired_server)
      {
        response_body["message"] = "Successfully killed server!";
      }
      else
      {
        response_body["message"] = "Failed killing server.";
      }

      // Get status of every other server and report
      std::pair<bool, json> server_report = BackendInterface::send_status_request();
      server_report.second[to_string(request_body["server"])] = false;

      // If it succeded this will have info, otherwise empty.
      response_body["report"] = server_report.second;
    }
    else if (action.compare("on") == 0)
    {
      // Invoke shutdown command
      bool revive_desired_server = BackendInterface::send_revive_request(toggle_server_ip);

      // Set the message to succesfully shutdown
      if (revive_desired_server)
      {
        response_body["message"] = "Successfully revived server!";
      }
      else
      {
        response_body["message"] = "Failed reviving server.";
      }

      // Get status of every other server and report
      std::pair<bool, json> server_report = BackendInterface::send_status_request();
      server_report.second[to_string(request_body["server"])] = true;

      // If it succeded this will have info, otherwise empty.
      response_body["report"] = server_report.second;
    }
    else
    {
      response_body["message"] =
          "Somehow received a request for an action other than off/on. This shouldn't be possible.";
      return {false, response_body};
    }

    return {true, response_body};
  }

  /**
   * @brief Get the colums & data for a specified row
   *
   * @param row - The row we want to retrieve data for
   * @return std::pair<bool, json> - Normal pair for api response to generate
   */
  std::pair<bool, json> get_row_data(json request_body)
  {
    json response_body;

    if (!request_body.contains("row"))
    {
      response_body["message"] = "Somehow missing specified row.";
      return {false, response_body};
    }

    std::string row = request_body["row"];

    // Since we don't have a RPC to get a specific row get everything's data
    std::pair<bool, json> all_rows_data = get_all_rows();

    // If the call was sucessful we can view the data for the row
    if (all_rows_data.first)
    {
      // If the row is no longer present for some reason return that it wasn't found
      if (!all_rows_data.second.contains(row))
      {
        response_body["message"] = "Row no longer found.";
        return {false, response_body};
      }

      // Return the json of columns & data for the specific row
      response_body = all_rows_data.second["row"];
      return {true, response_body};
    }

    // If the call wasn't succesful return that we couldn't get the data
    response_body["message"] = "Could not retrieve key-value stored data.";
    return {false, response_body};
  }
};

#endif