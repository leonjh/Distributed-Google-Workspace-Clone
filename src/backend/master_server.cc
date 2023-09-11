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

#include <algorithm>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <string.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
using backend::GetTabletsServerRequest;
using backend::GetTabletsUserRequest;
using backend::GetTabletsUserResponse;
using backend::IsInitialStorageServerStartupRequest;
using backend::IsInitialStorageServerStartupResponse;
using backend::KillStorageServerRequest;
using backend::MasterEmpty;
using backend::MasterGetKVStoreStateResponse;
using backend::MasterServer;
using backend::PushStorageTabletsRequest;
using backend::StorageEmpty;
using backend::StorageGetKVStoreStateRequest;
using backend::StorageGetKVStoreStateResponse;
using backend::StorageServer;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::CreateCustomChannel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

const int MAX_MESSAGE_SIZE = 1610612736; // 1.5 Gb

const int number_of_replicas = 3;
// const std::vector<std::string> replica_addresses = {
//     "127.0.0.1:8001", "127.0.0.1:8002", "127.0.0.1:8003", "127.0.0.1:8004",
//     "127.0.0.1:8005", "127.0.0.1:8006", "127.0.0.1:8007", "127.0.0.1:8008",
// };
const std::vector<std::string> replica_addresses = {
    "127.0.0.1:8001", "127.0.0.1:8002", "127.0.0.1:8003", "127.0.0.1:8004"
    // "127.0.0.1:8004"
};
// Logic and data behind the server's behavior.
class MasterServerServiceImpl final : public MasterServer::Service
{
private:
  struct Range
  {
    std::string begin;
    std::string end;
  };

  struct ServerMetadata
  {
    std::string address;
    bool primary;
  };

  struct Tablet
  {
    Range range;
    std::vector<ServerMetadata> replicas;
  };

  std::vector<Range> all_ranges;
  std::vector<Tablet> tablets;
  // map of address to tablets it is responsible for
  std::unordered_map<std::string, std::vector<int>> lookup_replica;
  std::unordered_map<std::string, bool> replica_alive;
  std::unordered_map<std::string, bool> replica_initial_startup;
  mutable std::shared_mutex tablet_lock;

public:
  MasterServerServiceImpl() = delete;

  MasterServerServiceImpl(MasterServerServiceImpl &) = delete;

  MasterServerServiceImpl(const std::vector<std::string> &addresses, int number_of_replicas)
  {
    partition_tablets(addresses, number_of_replicas);

    for (std::string address : addresses)
    {
      replica_initial_startup[address] = true;
    }
  }

  void partition_tablets(const std::vector<std::string> &addresses, int number_of_replicas)
  {
    /**
     * For now, configure tablets by first letter of row and randomly choose replicas
     */
    int ct = 0;
    std::string curr(2, ' ');
    for (int i = 0; i < 26; ++i)
    {
      for (int j = 0; j < 26; ++j)
      {
        curr[0] = i + 'a';
        curr[1] = j + 'a';
        if (all_ranges.size())
          all_ranges.back().end = curr;
        if (ct % 10 == 0)
        {
          all_ranges.push_back({curr, curr});
        }
        ct++;
      }
    }
    for (int i = 0; i < all_ranges.size(); i++)
    {
      std::vector<ServerMetadata> replicas;
      std::vector<int> used;
      for (int j = 0; j < number_of_replicas; ++j)
      {
        int index = rand() % addresses.size();
        while (std::find(used.begin(), used.end(), index) != used.end())
        {
          index = rand() % addresses.size();
        }
        used.push_back(index);
        if (j == 0)
        {
          replicas.push_back({addresses[index], true});
        }
        else
        {
          replicas.push_back({addresses[index], false});
        }
      }

      // std::cout << i << " " << all_ranges[i].begin << " " << all_ranges[i].end << std::endl;
      // for (int i = 0; i < replicas.size(); ++i)
      // {
      //   std::cout << replicas[i].address << " " << replicas[i].primary << std::endl;
      // }

      Tablet tablet = {all_ranges[i].begin, all_ranges[i].end, replicas};
      tablets.push_back(tablet);
      for (int j = 0; j < used.size(); ++j)
      {
        lookup_replica[addresses[used[j]]].push_back(tablets.size() - 1); // index
      }
    }

    print_tablet_assignments();
  }

  Status GetTabletsForUser(ServerContext *context, const GetTabletsUserRequest *request,
                           GetTabletsUserResponse *response) override
  {
    std::string key = request->key();
    std::cout << "get tablet groups for user: " << key << std::endl;

    int ind = 0;
    for (int i = 0; i < all_ranges.size(); ++i)
    {
      if (strcmp(all_ranges[i].begin.c_str(), key.c_str()) <= 0)
      {
        ind = i;
      }
    }
    Tablet tablet = tablets[ind];
    backend::GetTabletsUserResponse::TabletGroup *group = response->add_tablets();
    group->set_begin(tablet.range.begin);
    group->set_end(tablet.range.end);

    for (int i = 0; i < tablet.replicas.size(); ++i)
    {
      backend::GetTabletsUserResponse::TabletGroup::ServerInfo *info = group->add_servers();
      info->set_address(tablet.replicas[i].address);
    }

    return Status::OK;
  }

  /**
   * Construct a gRPC message which contains the local metadata about a particular tablet.
   * Forward this information to the list of recipients.
   */
  void send_storage_tablet_metadata(std::vector<std::string> &recipients)
  {
    std::shared_lock lock(tablet_lock);
    for (auto &address : recipients)
    {
      PushStorageTabletsRequest storage_request;
      StorageServerResponse storage_response;

      ClientContext context;
      for (int pos : lookup_replica[address])
      {
        auto tablet = tablets[pos];
        backend::PushStorageTabletsRequest::TabletGroup *group = storage_request.add_tablets();
        group->set_begin(tablet.range.begin);
        group->set_end(tablet.range.end);
        for (int i = 0; i < tablet.replicas.size(); ++i)
        {
          backend::PushStorageTabletsRequest::TabletGroup::ServerInfo *info = group->add_servers();
          info->set_address(tablet.replicas[i].address);
          info->set_primary(tablet.replicas[i].primary);
        }
      }

      grpc::ChannelArguments channel_args;
      channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);
      channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);

      std::unique_ptr<StorageServer::Stub> stub_(
          StorageServer::NewStub(grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), channel_args)));
      Status status = stub_->PushStorageTablets(&context, storage_request, &storage_response);
    }
  }

  void print_tablet_assignments()
  {
    for (int i = 0; i < tablets.size(); i++)
    {
      std::string tablet_range = tablets[i].range.begin + "," + tablets[i].range.end;
      std::cout << i << " " << tablet_range << std::endl;

      std::string primary_addr;
      for (int j = 0; j < tablets[i].replicas.size(); j++)
      {
        if (tablets[i].replicas[j].primary)
        {
          primary_addr = tablets[i].replicas[j].address;
        }
      }
      std::cout << primary_addr << " 1" << std::endl;
      for (int j = 0; j < tablets[i].replicas.size(); j++)
      {
        if (!tablets[i].replicas[j].primary)
        {
          std::cout << tablets[i].replicas[j].address << " 0" << std::endl;
        }
      }
    }
  }

  void primary_up(const std::string &curr_server_address)
  {
    replica_alive[curr_server_address] = true;

    std::cout << "PRIMARY UP: " << curr_server_address << std::endl;

    { // Case where node was killed and immediate revived after so no reassignments happened
      std::shared_lock lock(tablet_lock);
      for (int pos : lookup_replica[curr_server_address])
      {
        for (int i = 0; i < tablets[pos].replicas.size(); ++i)
        {
          if (tablets[pos].replicas[i].address == curr_server_address && tablets[pos].replicas[i].primary)
          {
            return;
          }
        }
      }
    }
    std::cout << "Performing reassignment" << std::endl;
    std::unique_lock lock(tablet_lock);
    for (int tablet_index : lookup_replica[curr_server_address])
    {
      // iterate through tablets curr server is responsible for
      std::vector<int> options;
      for (int i = 0; i < tablets[tablet_index].replicas.size(); ++i)
      {
        tablets[tablet_index].replicas[i].primary = false;
        if (replica_alive[tablets[tablet_index].replicas[i].address])
        {
          options.push_back(i);
        }
      }
      int random_new_primary = options[rand() % options.size()];
      tablets[tablet_index].replicas[random_new_primary].primary = true;
    }

    print_tablet_assignments();
  }

  void primary_down(const std::string &curr_server_address)
  {
    if (!replica_alive[curr_server_address])
    {
      // no need to return if already assigned appropriately
      return;
    }
    replica_alive[curr_server_address] = false;

    std::cout << "PRIMARY DOWN: " << curr_server_address << std::endl;
    std::unique_lock lock(tablet_lock);
    for (int tablet_index : lookup_replica[curr_server_address])
    {
      // iterate through tablets server is responsible for
      int problem = -1;
      std::vector<int> options;
      for (int i = 0; i < tablets[tablet_index].replicas.size(); ++i)
      {
        // iterate through the replicas for a tablet
        if (tablets[tablet_index].replicas[i].address == curr_server_address &&
            tablets[tablet_index].replicas[i].primary)
        {
          // note down if primary for tablet group is this server
          problem = i;
        }
        else if (tablets[tablet_index].replicas[i].address != curr_server_address &&
                 replica_alive[tablets[tablet_index].replicas[i].address])
        {
          // note down replicas in group which are still alive
          options.push_back(i);
        }
      }
      if (problem != -1)
      {
        // if curr server was primary for the group, reassign who is primary
        std::cout << "options size: " << options.size() << std::endl;
        int random_index = options[rand() % options.size()];
        tablets[tablet_index].replicas[problem].primary = false;
        tablets[tablet_index].replicas[random_index].primary = true;
      }
    }

    print_tablet_assignments();
  }

  Status GetTabletsForServer(ServerContext *context, const GetTabletsServerRequest *request,
                             MasterEmpty *response) override
  {
    std::string address = request->address();

    std::cout << "get tablet groups for server: " << address << std::endl;

    std::vector<std::string> recipients = {address};
    if (request->has_primary_up())
    {
      std::string primary = request->primary_up();
      primary_up(primary);
      recipients = replica_addresses;
    }
    else if (request->has_primary_down())
    {
      std::string primary = request->primary_down();
      // if it's already marked dead, no need to update assignments
      if (replica_alive[primary])
      {
        primary_down(primary);
        recipients = replica_addresses;
      }
    }

    send_storage_tablet_metadata(recipients);

    return Status::OK;
  }

  Status KillStorageServer(ServerContext *context, const KillStorageServerRequest *request,
                           MasterEmpty *response) override
  {
    std::string address = request->address();

    std::cout << "killing storage server " << address << std::endl;

    StorageEmpty storage_request;
    StorageEmpty storage_response;
    ClientContext storage_context;

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::unique_ptr<StorageServer::Stub> stub_(
        StorageServer::NewStub(grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), channel_args)));
    Status status = stub_->KillServer(&storage_context, storage_request, &storage_response);

    return Status::OK;
  }

  Status ReviveStorageServer(ServerContext *context, const KillStorageServerRequest *request,
                             MasterEmpty *response) override
  {
    std::string address = request->address();

    std::cout << "reviving storage server " << address << std::endl;

    StorageEmpty storage_request;
    StorageEmpty storage_response;
    ClientContext storage_context;

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::unique_ptr<StorageServer::Stub> stub_(
        StorageServer::NewStub(grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), channel_args)));
    Status status = stub_->ReviveServer(&storage_context, storage_request, &storage_response);

    return Status::OK;
  }

  Status GetKVStoreState(ServerContext *context, const MasterEmpty *request,
                         MasterGetKVStoreStateResponse *response) override
  {
    std::cout << "get kv store state: " << std::endl;
    std::map<std::string, std::map<std::string, std::string>> tmp_store;
    std::vector<std::pair<std::string, std::vector<std::string>>> addresses; // begin, address
    {
      std::shared_lock lock(tablet_lock);
      for (auto &tablet : tablets)
      {
        std::vector<std::string> vec;
        for (int i = 0; i < tablet.replicas.size(); ++i)
        {
          vec.push_back(tablet.replicas[i].address);
        }
        addresses.push_back({tablet.range.begin, std::move(vec)});
      }
    }
    for (auto &p : addresses)
    {
      bool found = 0;
      StorageGetKVStoreStateResponse storage_response;
      for (int i = 0; i < p.second.size(); ++i)
      {
        StorageGetKVStoreStateRequest storage_request;
        ClientContext storage_context;
        storage_request.set_key(p.first);

        grpc::ChannelArguments channel_args;
        channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

        channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
        //

        std::unique_ptr<StorageServer::Stub> stub_(StorageServer::NewStub(
            grpc::CreateCustomChannel(p.second[i], grpc::InsecureChannelCredentials(), channel_args)));
        Status status = stub_->GetKVStoreState(&storage_context, storage_request, &storage_response);
        if (status.ok())
        {
          found = 1;
          break;
        }
      }
      if (!found)
      {
        continue;
      }
      for (int i = 0; i < storage_response.rows_size(); ++i)
      {
        for (int j = 0; j < storage_response.rows(i).columns_size(); ++j)
        {
          tmp_store[storage_response.rows(i).name()][storage_response.rows(i).columns(j).name()] =
              storage_response.rows(i).columns(j).data();
        }
      }
    }
    for (auto &row : tmp_store)
    {
      backend::MasterGetKVStoreStateResponse::Row *current_proto_row = response->add_rows();
      current_proto_row->set_name(row.first);
      for (auto &col : tmp_store[row.first])
      {
        backend::MasterGetKVStoreStateResponse::Row::Column *current_proto_column = current_proto_row->add_columns();
        current_proto_column->set_name(col.first);
        current_proto_column->set_data(col.second);
      }
    }
    return Status::OK;
  }

  Status IsInitialStorageServerStartup(ServerContext *context, const IsInitialStorageServerStartupRequest *request,
                                       IsInitialStorageServerStartupResponse *response) override
  {
    std::string address = request->address();

    std::cout << "address: " << address << " is initial? " << replica_initial_startup[address] << std::endl;

    if (replica_initial_startup[address])
    {
      replica_alive[address] = true;
    }

    response->set_is_initial(replica_initial_startup[address]);

    replica_initial_startup[address] = false;

    return Status::OK;
  }

  Status GetStorageServerStatuses(ServerContext *context, const GetStorageServerStatusesRequest *request,
                                  GetStorageServerStatusesResponse *response) override
  {
    for (auto &replica_address : replica_addresses)
    {
      StorageEmpty pulse_request;
      StorageEmpty pulse_response;
      ClientContext context;

      grpc::ChannelArguments channel_args;
      channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);
      channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);

      std::unique_ptr<StorageServer::Stub> stub_(StorageServer::NewStub(
          grpc::CreateCustomChannel(replica_address, grpc::InsecureChannelCredentials(), channel_args)));
      Status status = stub_->PulseCheck(&context, pulse_request, &pulse_response);
      // if (replica_alive[replica_address] && !status.ok())
      // {
      //   primary_down(replica_address);
      //   std::vector<std::string> recipients = replica_addresses;
      //   send_storage_tablet_metadata(recipients);
      // }
      // else if (!replica_alive[replica_address] && status.ok())
      // {
      //   // don't expect this case to ever happen restarting storage server calls primary up before readying up
      //   primary_up(replica_address);
      //   std::vector<std::string> recipients = replica_addresses;
      //   send_storage_tablet_metadata(recipients);
      // }

      //   std::cout << replica_address << " : " << replica_alive[replica_address] << std::endl;

      backend::GetStorageServerStatusesResponse::ServerStatus *current_proto_status = response->add_statuses();
      current_proto_status->set_address(replica_address);
      current_proto_status->set_is_alive(status.ok());
    }

    return Status::OK;
  }
};

void RunServer()
{
  std::string server_address("127.0.0.1:8000");
  MasterServerServiceImpl service{replica_addresses, number_of_replicas};

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Master listening on " << server_address << std::endl;

  builder.SetMaxSendMessageSize(1610612736);
  builder.SetMaxMessageSize(1610612736);
  builder.SetMaxReceiveMessageSize(1610612736);

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();

  return 0;
}
