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
#include <sstream>
#include <string>

#ifdef BAZEL_BUILD
#include "examples/protos/storageserver.grpc.pb.h"
#else
#include "storageserver.grpc.pb.h"
#endif

using backend::StorageEmpty;
using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class StorageClient
{
public:
  StorageClient(std::shared_ptr<Channel> channel) : stub_(StorageServer::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  void Put(const std::string &row, const std::string &col, const std::string &value)
  {
    std::cout << "Sending put " << row << " " << col << " " << value << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1(value);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StoragePut(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }

  void CPut(const std::string &row, const std::string &col, const std::string &value1, const std::string &value2)
  {
    std::cout << "Sending cput " << row << " " << col << " " << value1 << " " << value2 << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);
    request.set_value1(value1);
    request.set_value2(value2);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StorageCPut(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }

  void Delete(const std::string &row, const std::string &col)
  {
    std::cout << "Sending delete " << row << " " << col << std::endl;

    // Data we are sending to the server.
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->StorageDelete(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }

  std::string Get(const std::string &row, const std::string &col)
  {
    // Data we are sending to the server.
    std::cout << "Sending get " << row << " " << col << std::endl;
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    // Container for the data we expect from the server.
    StorageServerResponse response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
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
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }
  }

  void Kill()
  {
    // Data we are sending to the server.
    StorageEmpty request;

    // Container for the data we expect from the server.
    StorageEmpty response;

    // Context for the client. It could be used to convey extra information
    // to the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->KillServer(&context, request, &response);

    // Act upon its status.
    if (status.ok())
    {
      return;
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return;
    }
  }

private:
  std::unique_ptr<StorageServer::Stub> stub_;
};

int main(int argc, char **argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).

  if (argc != 2)
  {
    std::cout << "Did not specify port" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string address = "127.0.0.1:" + std::string(argv[1]);

  StorageClient sc(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

  std::string op;
  std::string row, col;

  const std::vector<std::string> commands = {
      "put zy c1 v1", "put zy c2 v2", "put zy c3 v3", "put zy c4 v4",   "put zy c5 v5",   "put zy c6 v6",
      "put zy c7 v7", "put zy c8 v8", "put zy c9 v9", "put zy c10 v10", "put zy c11 v11", "put zy c12 v12",
  };

  int i = 1;
  while (1)
  {
    // std::stringstream ss(commands[i]);
    std::string cmd = "put zy c" + std::to_string(i) + " v" + std::to_string(i);
    std::stringstream ss(cmd);

    std::cin >> op;
    ss >> op;
    if (op == "kill")
    {
      sc.Kill();
      continue;
    }
    // std::cin >> row >> col;
    ss >> row >> col;
    if (op == "get")
    {
      std::string reply = sc.Get(row, col);
      std::cout << "Got: " << reply << std::endl;
    }
    else if (op == "put")
    {
      std::string val;
      // std::cin >> val;
      ss >> val;
      // val = "test \n with \n newline";
      // val = "test";
      sc.Put(row, col, val);
    }
    else if (op == "cput")
    {
      std::string val1, val2;
      // std::cin >> val1 >> val2;
      ss >> val1 >> val2;
      sc.CPut(row, col, val1, val2);
    }
    else if (op == "delete")
    {
      sc.Delete(row, col);
    }
    else
    {
      continue;
    }

    i++;
  }

  return 0;
}
