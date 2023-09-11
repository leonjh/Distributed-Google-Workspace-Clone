#ifndef BACKEND_INTERFACE_H__
#define BACKEND_INTERFACE_H__
#include <fstream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>

#include "../lib/constants.h"
#include "definitions.hpp"
#include "json.hpp"
#include "utils.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/masterserver.grpc.pb.h"
#include "examples/protos/storageserver.grpc.pb.h"
#else
#include "masterserver.grpc.pb.h"
#include "storageserver.grpc.pb.h"
#endif

using backend::GetStorageServerStatusesRequest;
using backend::GetStorageServerStatusesResponse;
using backend::GetTabletsUserRequest;
using backend::GetTabletsUserResponse;
using backend::KillStorageServerRequest;
using backend::MasterEmpty;
using backend::MasterGetKVStoreStateResponse;
using backend::MasterServer;
using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::CreateCustomChannel;
using grpc::Status;

using json = nlohmann::json;

const int MAX_MESSAGE_SIZE = 1610612736; // 1.5 Gb

class BackendInterface
{
public:
  static std::unordered_map<std::string, std::vector<std::string>> address_cache;

  BackendInterface() {}

  static std::unique_ptr<StorageServer::Stub> find_channel(const std::string &row_name)
  {
    // TODO: based on all storage servers
    // prob shouldnt create channel everytime. store all channels and pick.
    if (address_cache.find(row_name) == address_cache.end())
    {
      GetTabletsUserRequest request;
      request.set_key(row_name);

      // Container for the data we expect from the server.
      GetTabletsUserResponse response;

      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      find_master_channel()->GetTabletsForUser(&context, request, &response);

      for (int i = 0; i < response.tablets_size(); ++i)
      {
        for (int j = 0; j < response.tablets(i).servers_size(); ++j)
        {
          address_cache[row_name].push_back(response.tablets(i).servers(j).address());
        }
      }
    }

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::vector<std::string> &tmp = address_cache[row_name];
    return StorageServer::NewStub(
        grpc::CreateCustomChannel(tmp[rand() % tmp.size()], grpc::InsecureChannelCredentials(), channel_args));
  }

  /**
   * @brief Creates a stub to talk to the master server
   *
   * @return std::unique_ptr<MasterServer::Stub> - The master server stub
   */
  static std::unique_ptr<MasterServer::Stub> find_master_channel()
  {
    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    return MasterServer::NewStub(
        grpc::CreateCustomChannel("127.0.0.1:8000", grpc::InsecureChannelCredentials(), channel_args));
  }

  static bool send_put_request(const std::string &row, const std::string &col, const std::string &value)
  {
    std::cout << "put: " << row << " " << col << " " << value.substr(0, 1000) << std::endl;
    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1(value);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StoragePut(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "put: success" << std::endl;
        return true;
      }
      else
      {
        std::cout << "RPC failed, trying a different server" << std::endl;
      }
    }
    return false;
  }

  static bool send_put_bytes_request(const std::string &row, const std::string &col, const std::string &value)
  {
    std::cout << "put bytes: " << row << " " << col << " " << value.substr(0, 1000) << std::endl;
    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1_bytes(value);
    request.set_is_bytes(true);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StoragePut(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "put bytes: success" << std::endl;
        return true;
      }
      else
      {
        std::cout << status.error_message() << std::endl;
        std::cout << "RPC failed, trying a different server" << std::endl;
      }
    }
    return false;
  }

  static std::pair<bool, std::string> send_get_request(const std::string &row, const std::string &col)
  {
    std::cout << "get: " << row << " " << col << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StorageGet(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "get: success" << std::endl;
        return {true, response.value()};
      }
      else
      {
        std::cout << std::string(status.error_message()) << std::endl;

        if (status.error_code() == grpc::StatusCode::NOT_FOUND)
        {
          std::cout << "get: not found" << std::endl;
          return {false, "not found"};
        }
        else
        {
          std::cout << "RPC failed, trying a different server" << std::endl;
        }
      }
    }
    return {false, "RPC failed"};
  }

  static std::pair<bool, std::string> send_get_bytes_request(const std::string &row, const std::string &col)
  {
    std::cout << "get bytes: " << row << " " << col << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_is_bytes(true);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StorageGet(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "get bytes: success" << std::endl;
        return {true, response.value_bytes()};
      }
      else
      {
        std::cout << std::string(status.error_message()) << std::endl;

        if (status.error_code() == grpc::StatusCode::NOT_FOUND)
        {
          std::cout << "get: not found" << std::endl;
          return {false, "not found"};
        }
        else
        {
          std::cout << "RPC failed, trying a different server" << std::endl;
        }
      }
    }
    return {false, "RPC failed"};
  }

  static bool send_delete_request(const std::string &row, const std::string &col)
  {
    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information to
      // the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StorageDelete(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "put: success" << std::endl;
        return true;
      }
      else
      {
        std::cout << "RPC failed, trying a different server" << std::endl;
      }
    }
    return false;
  }

  static bool send_cput_request(const std::string &row, const std::string &col, const std::string &value1,
                                const std::string &value2)
  {
    std::cout << "cput: " << row << " " << col << " " << value1.substr(0, 1000) << " " << value2.substr(0, 1000)
              << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1(value1);
    request.set_value2(value2);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    int tries = 50;
    while (tries--)
    {
      // Context for the client. It could be used to convey extra information
      // to the server and/or tweak certain RPC behaviors.
      ClientContext context;

      // The actual RPC.
      Status status = find_channel(row)->StorageCPut(&context, request, &response);

      // Act upon its status.
      if (status.ok())
      {
        std::cout << "put: success" << std::endl;
        return true;
      }
      else
      {
        std::cout << "RPC failed, trying a different server" << std::endl;
      }
    }
    return false;
  }

  static bool send_kill_request(const std::string &ip)
  {
    // Data we are sending to the server.
    KillStorageServerRequest request;
    request.set_address(ip);

    // Container for the data we expect from the server.
    MasterEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = find_master_channel()->KillStorageServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return true;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return false;
    }
  }

  /**
   * @brief Sends a storage server revive request to the master server
   *
   * @param ip - The ip of the storage server to kill
   * @return true - The server was successfully killed
   * @return false  - The server could not be killed (RPC failure and/or any other reason)
   */
  static bool send_revive_request(const std::string &ip)
  {
    // Data we are sending to the server.
    KillStorageServerRequest request;
    request.set_address(ip);

    // Container for the data we expect from the server.
    MasterEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = find_master_channel()->ReviveStorageServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return true;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return false;
    }
  }

  /**
   * @brief Checks on the alive/dead status of every storage server and
   * returns a json for the frontend to work off of
   *
   * @return std::pair<bool, json> - 1. true/false if the call succedes/fails, json if it worked
   */
  static std::pair<bool, json> send_status_request()
  {
    GetStorageServerStatusesRequest request;
    GetStorageServerStatusesResponse response;
    ClientContext context;

    // The actual RPC.
    Status status = find_master_channel()->GetStorageServerStatuses(&context, request, &response);
    json server_statuses;
    if (status.ok())
    {
      std::cout << "PRINTING STATUSES" << std::endl;
      for (int i = 0; i < response.statuses_size(); ++i)
      {
        std::string state = response.statuses(i).is_alive() ? "alive" : "dead";
        std::cout << response.statuses(i).address() << " " << state << std::endl;

        server_statuses[std::to_string(i + 1)] = response.statuses(i).is_alive();
      }
      return {true, server_statuses};
    }
    else
    {
      std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
      return {false, server_statuses};
    }
  }

  /**
   * @brief Get the kv state of each row & column
   *
   * @return std::pair<bool, json>
   */
  static std::pair<bool, json> get_kv_state()
  {
    // Data we are sending to the server.
    MasterEmpty request;

    // Container for the data we expect from the server.
    MasterGetKVStoreStateResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = find_master_channel()->GetKVStoreState(&context, request, &response);

    // The response json
    json response_json;

    // Act upon its status.
    if (status.ok())
    {
      // std::cout << "PRINTING STATE" << std::endl;
      for (int i = 0; i < response.rows_size(); ++i)
      {
        // std::cout << "ROW: " << response.rows(i).name() << std::endl;
        std::vector<json> columns_for_row;

        for (int j = 0; j < response.rows(i).columns_size(); ++j)
        {
          json column_and_data;
          column_and_data[response.rows(i).columns(j).name()] = response.rows(i).columns(j).data();
          // std::cout << response.rows(i).columns(j).name() << ": " << response.rows(i).columns(j).data() << std::endl;
          columns_for_row.push_back(column_and_data);
        }
        json row_data(columns_for_row);

        response_json[response.rows(i).name()] = row_data;
        // std::cout << std::endl;
      }

      return {true, response_json};
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    }
    return {false, response_json};
  }
};

#endif