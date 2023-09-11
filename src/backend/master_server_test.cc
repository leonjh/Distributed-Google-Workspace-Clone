/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#ifdef BAZEL_BUILD
#include "examples/protos/masterserver.grpc.pb.h"
#else
#include "masterserver.grpc.pb.h"
#endif

using backend::GetStorageServerStatusesRequest;
using backend::GetStorageServerStatusesResponse;
using backend::GetTabletsServerRequest;
using backend::GetTabletsUserRequest;
using backend::GetTabletsUserResponse;
using backend::KillStorageServerRequest;
using backend::MasterEmpty;
using backend::MasterGetKVStoreStateResponse;
using backend::MasterServer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class MasterClient
{
public:
  MasterClient(std::shared_ptr<Channel> channel) : stub_(MasterServer::NewStub(channel)) {}

  void GetTabletsForUser(const std::string &key)
  {
    // Data we are sending to the server.
    GetTabletsUserRequest request;
    request.set_key(key);

    // Container for the data we expect from the server.
    GetTabletsUserResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetTabletsForUser(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      print_response(response);
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

  void GetTabletsForServer(const std::string &key)
  {
    // Data we are sending to the server.
    GetTabletsServerRequest request;
    request.set_address(key);

    // Container for the data we expect from the server.
    MasterEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetTabletsForServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

  void kill_server(const std::string &key)
  {
    // Data we are sending to the server.
    KillStorageServerRequest request;
    request.set_address(key);

    // Container for the data we expect from the server.
    MasterEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->KillStorageServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

  void revive_server(const std::string &key)
  {
    // Data we are sending to the server.
    KillStorageServerRequest request;
    request.set_address(key);

    // Container for the data we expect from the server.
    MasterEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->ReviveStorageServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

  void get_state()
  {
    // Data we are sending to the server.
    MasterEmpty request;

    // Container for the data we expect from the server.
    MasterGetKVStoreStateResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetKVStoreState(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      std::cout << "PRINTING STATE" << std::endl;
      for (int i = 0; i < response.rows_size(); ++i)
      {
        std::cout << "ROW: " << response.rows(i).name() << std::endl;
        for (int j = 0; j < response.rows(i).columns_size(); ++j)
        {
          std::cout << response.rows(i).columns(j).name() << ": " << response.rows(i).columns(j).data() << std::endl;
        }
        std::cout << std::endl;
      }
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

  void get_statuses()
  {
    GetStorageServerStatusesRequest request;

    GetStorageServerStatusesResponse response;

    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetStorageServerStatuses(&context, request, &response);

    if (status.ok())
    {
      std::cout << "PRINTING STATUSES" << std::endl;
      for (int i = 0; i < response.statuses_size(); ++i)
      {
        std::string state = response.statuses(i).is_alive() ? "alive" : "dead";
        std::cout << response.statuses(i).address() << " " << state << std::endl;
      }
    }
    else
    {
      std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }

private:
  std::unique_ptr<MasterServer::Stub> stub_;

  void print_response(const GetTabletsUserResponse &response)
  {
    std::cout << "Size is " << response.tablets_size() << std::endl;
    for (int i = 0; i < response.tablets_size(); ++i)
    {
      std::cout << response.tablets(i).begin() << " " << response.tablets(i).end() << std::endl;
      for (int j = 0; j < response.tablets(i).servers_size(); ++j)
      {
        std::cout << response.tablets(i).servers(j).address() << std::endl;
      }
      std::cout << std::endl;
    }
  }
};

int main(int argc, char **argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1)
  {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos)
    {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=')
      {
        target_str = arg_val.substr(start_pos + 1);
      }
      else
      {
        std::cout << "The only correct argument syntax is --target=" << std::endl;
        return 0;
      }
    }
    else
    {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  }
  else
  {
    target_str = "127.0.0.1:8000";
  }
  MasterClient mc(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  std::string op, key;
  while (1)
  {
    std::cin >> op;
    if (op == "state")
    {
      mc.get_state();
      continue;
    }
    else if (op == "statuses")
    {
      mc.get_statuses();
      continue;
    }
    std::cin >> key;
    if (op == "server") // specify ip address
    {
      mc.GetTabletsForServer(key);
    }
    else if (op == "kill")
    {
      mc.kill_server(key);
    }
    else if (op == "revive")
    {
      mc.revive_server(key);
    }
    else // specify key
    {
      mc.GetTabletsForUser(key);
    }
  }
  return 0;
}
