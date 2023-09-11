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

#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>

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

#define MAX_LOG_LENGTH 128
#define MAX_GRPC_MSG_SIZE 12000000   // 12 MB
#define MAX_LOG_FILE_SIZE 1610612736 //
#define MAX_CP_STREAM_SIZE 10000     // too large -> segfault
#define MAX_LOG_CMD_COUNT 5

using backend::GetTabletsServerRequest;
using backend::IsInitialStorageServerStartupRequest;
using backend::IsInitialStorageServerStartupResponse;
using backend::MasterEmpty;
using backend::MasterServer;
using backend::PushCheckpointPartitionRequest;
using backend::PushCheckpointPartitionResponse;
using backend::PushLogRequest;
using backend::PushLogResponse;
using backend::PushStorageTabletsRequest;
using backend::RequestLatestTabletStateRequest;
using backend::RequestLatestTabletStateResponse;
using backend::StorageEmpty;
using backend::StorageGetKVStoreStateRequest;
using backend::StorageGetKVStoreStateResponse;
using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::CreateCustomChannel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;

const int MAX_MESSAGE_SIZE = 1610612736; // 1.5 Gb

// Logic and data behind the server's behavior.
class StorageServerImpl final : public StorageServer::Service
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

  struct Entry
  {
    std::string value;
    bool is_bytes;
  };

  std::vector<Tablet> tablets;
  // shared lock to read tablet table (tablets vector)
  std::shared_mutex tablet_metadata_lock;

  const int MAX_LOG_ENTRIES = 4;
  const std::string master_address = "127.0.0.1:8000";
  const std::string my_address;
  std::string BACKUP_FILEPATH;
  // index of tablet in tablets -> row -> col -> value
  std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<std::string, Entry>>> tablet_to_kv_store;
  // std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
  //     tablet_to_kv_store;
  std::unordered_map<int, std::shared_mutex> tablet_locks;
  // index of tablet -> kv tablet locks lock
  // std::unordered_map<int, std::mutex> kv_tablet_locks;
  std::unordered_map<std::string, int> test_map;
  // index of tablet -> log lock
  std::unordered_map<int, std::shared_mutex> log_locks;
  // index of tablet -> (number of entries in log, file size)
  std::unordered_map<int, std::pair<int, int>> tablet_log_size;
  // tablet index -> checkpoint version
  std::unordered_map<int, int> checkpoint_version;
  // map of row key to lock
  std::unordered_map<std::string, std::mutex> locks;
  std::unordered_map<std::string, std::condition_variable> cvs;
  // row key -> seq_num
  std::unordered_map<std::string, int> sequence_numbers;
  // row key -> max seq_num seen across all threads
  std::unordered_map<std::string, int> sequence_max_seen;
  std::mutex cout_mutex;

  bool alive = true;
  bool booting = true;

public:
  StorageServerImpl() = delete;

  StorageServerImpl(StorageServerImpl &) = delete;

  StorageServerImpl(const std::string &my_address) : my_address{my_address}
  {
    BACKUP_FILEPATH = "../backups/" + my_address + "/";
  }

  Status PushStorageTablets(ServerContext *context, const PushStorageTabletsRequest *request,
                            StorageServerResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }

    fill_tablets(request);

    return Status::OK;
  }

  Status StorageGet(ServerContext *context, const StorageServerRequest *request,
                    StorageServerResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
    if (booting)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "server is still booting");
    }

    std::string row = request->row();
    std::string col = request->col();

    log_debug("received GET request (row: %s) (col: %s)", row.c_str(), col.c_str());
    {
      std::lock_guard<std::mutex> lock(locks[row]);

      int tablet_index = get_tablet(row);
      if (tablet_index == -1)
      {
        return Status(grpc::StatusCode::NOT_FOUND, "row not contained in any tablet managed by this server");
      }
      auto key_value_store = tablet_to_kv_store[tablet_index];
      // tablet_to_kv_store
      if (key_value_store.find(row) != key_value_store.end() &&
          key_value_store[row].find(col) != key_value_store[row].end())
      {
        if (request->has_is_bytes() && request->is_bytes())
        {
          response->set_value_bytes(key_value_store[row][col].value);
        }
        else
        {
          response->set_value(key_value_store[row][col].value);
        }
        return Status::OK;
      }
      else
      {
        return Status(grpc::StatusCode::NOT_FOUND, "no value found");
      }
    }
  }

  Status StoragePut(ServerContext *context, const StorageServerRequest *request,
                    StorageServerResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
    if (booting)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "server is still booting");
    }

    return handle_request(request, response, "PUT");
  }

  Status StorageCPut(ServerContext *context, const StorageServerRequest *request,
                     StorageServerResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
    if (booting)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "server is still booting");
    }

    return handle_request(request, response, "CPUT");
  }

  Status StorageDelete(ServerContext *context, const StorageServerRequest *request,
                       StorageServerResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
    if (booting)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "server is still booting");
    }

    return handle_request(request, response, "DELETE");
  }

  Status KillServer(ServerContext *context, const StorageEmpty *request, StorageEmpty *response) override
  {
    alive = false;
    std::cout << "im dead now" << std::endl;

    return Status::OK;
  }

  Status ReviveServer(ServerContext *context, const StorageEmpty *request, StorageEmpty *response) override
  {
    if (!alive)
    {
      std::cout << "revived" << std::endl;
      alive = true;

      bootstrap_server();
    }

    return Status::OK;
  }

  Status PushCheckpointPartition(ServerContext *context, ServerReader<PushCheckpointPartitionRequest> *reader,
                                 PushCheckpointPartitionResponse *response) override
  {
    // Accumulate the request data from the incoming request stream
    PushCheckpointPartitionRequest request;

    std::string checkpoint_path;
    std::ofstream ofs;
    std::string tablet_range;
    int checkpoint_version = 0;
    while (reader->Read(&request))
    {
      if (checkpoint_path.size() == 0)
      {
        checkpoint_version = request.checkpoint_version();
        tablet_range = request.tablet_range();

        log_debug_with_tablet(tablet_range, "STARTING retrieving checkpoint %d from primary", checkpoint_version);

        checkpoint_path = BACKUP_FILEPATH + "checkpoints/" + tablet_range + "/" + std::to_string(checkpoint_version);
        std::to_string(request.checkpoint_version());
        ofs.open(checkpoint_path, std::ios::out | std::ios::binary);
        if (!ofs)
        {
          std::cerr << "unable to create a file for the checkpoint" << std::endl;
          return Status(grpc::StatusCode::INTERNAL, "unable to create a file for the checkpoint");
        }
      }

      std::cout << "length of content: " << request.content().length() << std::endl;

      // ofs << request.content();
      ofs.write(request.content().c_str(), request.content().length());
    }
    ofs.close();
    log_debug_with_tablet(tablet_range, "FINISHED retrieving checkpoint %d from primary", checkpoint_version);

    return Status::OK;
  }

  Status PushLog(ServerContext *context, const PushLogRequest *request, PushLogResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }

    int tablet_index = get_tablet(request->tablet_range());

    std::lock_guard<std::shared_mutex> lock(log_locks[tablet_index]);
    std::string log_path = BACKUP_FILEPATH + "logs/" + request->tablet_range();
    std::ofstream log(log_path, std::ios::trunc);
    if (!log.is_open())
    {
      log_error_with_tablet(request->tablet_range(), "unable to create a log file at path (%s)", log_path);
      return Status(grpc::StatusCode::INTERNAL, "unable to create a log file");
    }

    log << request->content();
    log.close();

    return Status::OK;
  }

  Status RequestLatestTabletState(ServerContext *context, const RequestLatestTabletStateRequest *request,
                                  RequestLatestTabletStateResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }

    std::string s_tablet_range = request->tablet_range();
    int s_checkpoint_version = request->checkpoint_version();
    int s_log_length = request->log_length();
    std::string s_address = request->address();

    // TODO: acquire shared tablet lock to prevent writes but still allow reads
    // unique lock on tablet here
    // shared lock on every write operation
    // note that all writes should acquire an exclusive tablet lock
    // challenge: allow puts on different rows in same tablet but no operation on tablet while this is going on
    // maybe have a second lock vector called tablet_freeze_locks (ops acquire shared locks on these while this op
    // acquires exclusive lock)

    int tablet_index = get_tablet(s_tablet_range);

    bool checkpoint_needed = false;
    bool log_needed = false;

    if (s_checkpoint_version == checkpoint_version[tablet_index])
    {
      if (checkpoint_version[tablet_index] == 0)
      {
        // no checkpoint currently available
        checkpoint_needed = false;
      }
      if (s_log_length != tablet_log_size[tablet_index].first)
      {
        // checkpoints match but logs don't
        log_needed = true;
      }
    }
    else
    {
      // checkpoints don't match => checkpoint and log needed
      checkpoint_needed = true;
      log_needed = true;
    }

    // create new method for this
    // can take new writes up to the point where new checkpoint would occur
    if (checkpoint_needed)
    {
      log_debug_with_tablet(s_tablet_range, "sending checkpoint %d to %s", checkpoint_version[tablet_index],
                            s_address.c_str());

      grpc::ChannelArguments channel_args;
      channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

      channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
      //

      std::unique_ptr<StorageServer::Stub> stub_(StorageServer::NewStub(
          grpc::CreateCustomChannel(s_address, grpc::InsecureChannelCredentials(), channel_args)));

      ClientContext cp_context;
      PushCheckpointPartitionResponse cp_response;
      auto stream_writer = stub_->PushCheckpointPartition(&cp_context, &cp_response);

      // read last checkpoint file line-by-line
      std::string checkpoint_path =
          BACKUP_FILEPATH + "checkpoints/" + s_tablet_range + "/" + std::to_string(checkpoint_version[tablet_index]);
      // std::ifstream infile(checkpoint_path, std::ios_base::in | std::ios::binary);
      // if (!infile)
      // {
      //   std::cerr << "Failed to open checkpoint" << std::endl;
      // }

      // no more than 1 line more than this
      const int MAX_CHECKPOINT_PARTITION_LENGTH = 10;
      std::string checkpoint_content;
      std::string line;
      bool line_written = false;

      // write by character count

      // MAX_CP_STREAM_SIZE
      // char buffer[MAX_CP_STREAM_SIZE + 1];
      // int count = 0;
      // while (infile.read(buffer, MAX_CP_STREAM_SIZE))
      // {
      //   buffer[infile.gcount()] = '\0';

      //   // process buffer
      //   PushCheckpointPartitionRequest cp_request;
      //   cp_request.set_checkpoint_version(checkpoint_version[tablet_index]);
      //   cp_request.set_tablet_range(s_tablet_range);
      //   cp_request.set_content(buffer);

      //   stream_writer->Write(cp_request);
      //   count++;
      // }
      // std::cout << "Number of times it looped: " << count << std::endl;
      // if (infile.gcount() > 0)
      // {
      //   buffer[infile.gcount()] = '\0';

      //   PushCheckpointPartitionRequest cp_request;
      //   cp_request.set_checkpoint_version(checkpoint_version[tablet_index]);
      //   cp_request.set_tablet_range(s_tablet_range);
      //   cp_request.set_content(buffer);

      //   stream_writer->Write(cp_request);
      // }

      int fd = open(checkpoint_path.c_str(), O_RDONLY);
      if (fd == -1)
      {
        perror("Error opening file");
        exit(EXIT_FAILURE);
      }

      char buffer[MAX_CP_STREAM_SIZE + 1];
      ssize_t num_bytes_read;
      while ((num_bytes_read = read(fd, buffer, MAX_CP_STREAM_SIZE)) > 0)
      {
        PushCheckpointPartitionRequest cp_request;
        cp_request.set_checkpoint_version(checkpoint_version[tablet_index]);
        cp_request.set_tablet_range(s_tablet_range);
        cp_request.set_content(std::string(buffer, num_bytes_read));

        stream_writer->Write(cp_request);
      }
      close(fd);

      stream_writer->WritesDone();
      Status status = stream_writer->Finish();
    }

    if (log_needed)
    {
      ClientContext log_context;
      PushLogRequest push_log_request;
      PushLogResponse push_log_response;

      push_log_request.set_tablet_range(s_tablet_range);

      std::string log_path = BACKUP_FILEPATH + "logs/" + s_tablet_range;
      std::ifstream log(log_path);
      std::stringstream content;
      content << log.rdbuf();
      push_log_request.set_content(content.str());

      grpc::ChannelArguments channel_args;
      channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

      channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
      //

      std::unique_ptr<StorageServer::Stub> stub_(StorageServer::NewStub(
          grpc::CreateCustomChannel(s_address, grpc::InsecureChannelCredentials(), channel_args)));
      Status status = stub_->PushLog(&log_context, push_log_request, &push_log_response);
    }

    return Status::OK;
  }

  Status PulseCheck(ServerContext *context, const StorageEmpty *request, StorageEmpty *response) override
  {
    if (alive)
    {
      std::cout << "pulse check" << std::endl;

      return Status::OK;
    }
    else
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
  }

  int get_last_checkpoint(std::string tablet_range)
  {
    std::string checkpoint_dir = BACKUP_FILEPATH + "checkpoints/" + tablet_range;

    int max_checkpoint_version = 0;
    // Iterate over files in directory
    for (auto &file : std::filesystem::directory_iterator(checkpoint_dir))
    {
      // Check if file is a regular file
      if (std::filesystem::is_regular_file(file))
      {
        std::string checkpoint_name = file.path().filename().string();
        max_checkpoint_version = std::max(std::stoi(checkpoint_name), max_checkpoint_version);
      }
    }

    return max_checkpoint_version;
  }

  void sync_tablet_with_primary(std::string tablet_range)
  {
    RequestLatestTabletStateRequest request;
    RequestLatestTabletStateResponse response;

    int tablet_index = get_tablet(tablet_range);

    // retrieve last checkpoint version from disk
    int checkpoint_version = get_last_checkpoint(tablet_range);
    update_curr_log_size(tablet_index);
    // determine log size from disk
    int log_size = tablet_log_size[tablet_index].first;

    request.set_tablet_range(tablet_range);
    request.set_checkpoint_version(checkpoint_version);
    request.set_log_length(log_size);
    request.set_address(my_address);

    // get primary address from tablet list

    std::string primary_addr = get_ordered_replicas(tablet_index)[0];
    log_debug_with_tablet(tablet_range, "initiating synchronization with primary (%s)", primary_addr.c_str());

    if (my_address != primary_addr)
    {
      // retrieve latest tablet state from primary
      Status status;
      do
      {
        ClientContext context;

        grpc::ChannelArguments channel_args;
        channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

        channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
        //

        std::unique_ptr<StorageServer::Stub> stub_(StorageServer::NewStub(
            grpc::CreateCustomChannel(primary_addr, grpc::InsecureChannelCredentials(), channel_args)));
        status = stub_->RequestLatestTabletState(&context, request, &response);

        if (!status.ok())
        {
          log_debug_with_tablet(tablet_range, "current primary (%s) is down", primary_addr.c_str());
          get_tablets_metadata(primary_addr, false);
          primary_addr = get_ordered_replicas(tablet_index)[0];
          // master should verify himself that node is down - master shouldn't return until push tablet is done
          // note that multiple threads will likely get here at the same time
        }

      } while (!status.ok());
    }

    // copy of latest checkpoint should be on disk at this point (latest might already be on disk)
    checkpoint_version = get_last_checkpoint(tablet_range);
    if (checkpoint_version > 0)
    {
      load_checkpoint(tablet_index, checkpoint_version);
    }
    else
    {
      // log_debug_with_tablet(tablet_range, "no checkpoint to load");
    }

    // retrieve latest log if needed

    // apply log to tablet
    load_log(tablet_index);
  }

  void load_backup_initial(std::string tablet_range)
  {
    int tablet_index = get_tablet(tablet_range);

    // create log file for tablet if one doesn't exist
    std::string log_path = BACKUP_FILEPATH + "logs/" + tablet_range;
    if (!std::filesystem::exists(log_path))
    {
      std::ofstream ofs(log_path);
      ofs.close();
    }

    std::string checkpoint_dir = BACKUP_FILEPATH + "checkpoints/" + tablet_range;
    std::filesystem::create_directories(checkpoint_dir);

    int checkpoint_version = get_last_checkpoint(tablet_range);
    if (checkpoint_version > 0)
    {
      load_checkpoint(tablet_index, checkpoint_version);
    }

    load_log(tablet_index);
  }

  void bootstrap_server()
  {
    log_debug("initiating bootstrap of server");

    // load all tablet information from master

    // ask master if this is initial startup - master responds yes until it gets one yes from everyone after which it
    // will say no
    // TODO: change this to set via querying master
    ClientContext context;
    IsInitialStorageServerStartupRequest request;
    IsInitialStorageServerStartupResponse response;

    // initial startup when master thinks server is alive - doesn't know about death from ctrl c
    request.set_address(my_address);

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::unique_ptr<MasterServer::Stub> stub_(MasterServer::NewStub(
        grpc::CreateCustomChannel(master_address, grpc::InsecureChannelCredentials(), channel_args)));
    Status status = stub_->IsInitialStorageServerStartup(&context, request, &response);

    if (!status.ok())
    {
      std::cerr << "Unable to reach master" << std::endl;
      exit(0);
    }

    bool is_this_initial = response.is_initial();
    if (is_this_initial)
    {
      log_debug("bootstrap for first-time server start");
      get_tablets_metadata("", true);

      // create logs folder if it doesn't exist
      std::string log_dir = BACKUP_FILEPATH + "logs/";
      std::filesystem::create_directories(log_dir);

      // set checkpoint version and log size if they exist
      std::vector<std::thread> threads;
      for (int tablet_index = 0; tablet_index < tablets.size(); ++tablet_index)
      {
        std::string tablet_range = get_tablet_range(tablet_index);
        threads.emplace_back(&StorageServerImpl::load_backup_initial, this, tablet_range);
      }

      for (auto &thread : threads)
      {
        thread.join();
      }
      log_debug("all tablets have been loaded");
    }
    else
    {
      log_debug("bootstrap for server start after crash");
      // request tablet primary reassignment without me
      get_tablets_metadata(my_address, false);

      // set checkpoint version and log size if they exist
      std::vector<std::thread> threads;
      for (int tablet_index = 0; tablet_index < tablets.size(); ++tablet_index)
      {
        std::string tablet_range = get_tablet_range(tablet_index);
        threads.emplace_back(&StorageServerImpl::sync_tablet_with_primary, this, tablet_range);
      }

      for (auto &thread : threads)
      {
        thread.join();
      }

      log_debug("all tablets have been loaded");

      // std::string tablet_range = "zu_zz";
      // sync_tablet_with_primary(tablet_range);

      // reassign primaries to include me as a candidate
      get_tablets_metadata(my_address, true);
    }

    // TODO: notify master you are ready
    booting = false;
    log_debug("bootstrap complete");
  }

  void get_tablets_metadata(std::string primary, bool is_up)
  {
    GetTabletsServerRequest request;
    request.set_address(my_address);
    if (primary.size() > 0)
    {
      if (is_up)
      {
        request.set_primary_up(primary);
        log_debug("request tablet-server assignments now including server %s", primary.c_str());
      }
      else
      {
        request.set_primary_down(primary);
        log_debug("request tablet-server assignments excluding server %s", primary.c_str());
      }
    }
    else
    {
      log_debug("request tablet-server assignments");
    }

    if (primary.size() != 0)
    {
    }
    else
    {
    }

    MasterEmpty response;

    ClientContext context;

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::unique_ptr<MasterServer::Stub> stub_(MasterServer::NewStub(
        grpc::CreateCustomChannel(master_address, grpc::InsecureChannelCredentials(), channel_args)));
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

  Status GetKVStoreState(ServerContext *context, const StorageGetKVStoreStateRequest *request,
                         StorageGetKVStoreStateResponse *response) override
  {
    if (!alive)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "hi im dead");
    }
    if (booting)
    {
      return Status(grpc::StatusCode::UNAVAILABLE, "server is still booting");
    }

    std::string key = request->key();
    std::cout << "get kv store state: " << key << std::endl;
    int index = get_tablet(key);

    if (index == -1)
    {
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "Requested row not in range of this server");
    }
    std::string begin, end;
    {
      std::shared_lock lock(tablet_metadata_lock);
      begin = tablets[index].range.begin;
      end = tablets[index].range.end;
    }

    for (auto &tablet_kv_store_pair : tablet_to_kv_store)
    {
      auto &key_value_store = tablet_kv_store_pair.second;
      for (auto &row : key_value_store)
      {
        if (strcmp(begin.c_str(), row.first.c_str()) > 0 || strcmp(row.first.c_str(), end.c_str()) >= 0)
        {
          continue;
        }
        backend::StorageGetKVStoreStateResponse::Row *current_proto_row = response->add_rows();
        current_proto_row->set_name(row.first);
        for (auto &col : key_value_store[row.first])
        {
          backend::StorageGetKVStoreStateResponse::Row::Column *current_proto_column = current_proto_row->add_columns();
          current_proto_column->set_name(col.first);
          if (col.second.is_bytes)
          {
            current_proto_column->set_data("UTF-8 Encoded Data: No preview available.");
          }
          else
          {
            current_proto_column->set_data(col.second.value);
          }
        }
      }
    }

    return Status::OK;
  }

private:
  void execute_put(const StorageServerRequest *request, int tablet_index)
  {
    std::string row = request->row();
    std::string col = request->col();
    std::string value;
    bool is_bytes = false;
    if (request->has_is_bytes() && request->is_bytes())
    {
      value = request->value1_bytes();
      is_bytes = true;
    }
    else
    {
      value = request->value1();
    }
    int num = request->sequence_number();

    {
      std::unique_lock lock(locks[row]);
      sequence_numbers[row] = num;
      sequence_max_seen[row] = num;
      tablet_to_kv_store[tablet_index][row][col] = {value, is_bytes};
      cvs[row].notify_all();
    }
  }

  void execute_cput(const StorageServerRequest *request, int tablet_index)
  {
    std::string row = request->row();
    std::string col = request->col();
    std::string value1;
    bool is_bytes;
    if (request->has_is_bytes() && request->is_bytes())
    {
      value1 = request->value1_bytes();
      is_bytes = true;
    }
    else
    {
      value1 = request->value1();
    }
    std::string value2;
    if (request->has_is_bytes() && request->is_bytes())
    {
      value2 = request->value2_bytes();
    }
    else
    {
      value2 = request->value2();
    }
    int num = request->sequence_number();

    {
      std::unique_lock lock(locks[row]);
      sequence_numbers[row] = num;
      sequence_max_seen[row] = num;
      if (tablet_to_kv_store[tablet_index][row].find(col) != tablet_to_kv_store[tablet_index][row].end() &&
          tablet_to_kv_store[tablet_index][row][col].value == value1)
        tablet_to_kv_store[tablet_index][row][col] = {value2, is_bytes};
      cvs[row].notify_all();
    }
  }

  void execute_delete(const StorageServerRequest *request, int tablet_index)
  {
    std::string row = request->row();
    std::string col = request->col();
    int num = request->sequence_number();

    {
      std::unique_lock lock(locks[row]);
      sequence_numbers[row] = num;
      sequence_max_seen[row] = num;
      tablet_to_kv_store[tablet_index][row].erase(col);
      cvs[row].notify_all();
    }
  }

  std::string get_timestamp()
  {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    // Convert the time to a string with the format "YYYY-MM-DD HH:MM:SS"
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

    return std::string{timestamp, sizeof(timestamp) - 1};
  }

  void log_debug(const char *fmt, ...)
  {
    char buf[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::string timestamp = get_timestamp();

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "[" << timestamp << "]-" << my_address << ": " << buf << std::endl;
    }
  }

  void log_error(const char *fmt, ...)
  {
    char buf[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::string timestamp = get_timestamp();

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cerr << "[" << timestamp << "]-" << my_address << ": " << buf << std::endl;
    }
  }

  void log_debug_with_tablet(const std::string tablet, const char *fmt, ...)
  {
    char buf[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::string timestamp = get_timestamp();

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cout << "[" << timestamp << "]-" << my_address << "-" << tablet << ": " << buf << std::endl;
    }
  }

  void log_error_with_tablet(const std::string tablet, const char *fmt, ...)
  {
    char buf[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::string timestamp = get_timestamp();

    {
      std::lock_guard<std::mutex> lock(cout_mutex);
      std::cerr << "[" << timestamp << "]-" << my_address << "-" << tablet << ": " << buf << std::endl;
    }
  }

  std::string serialize_command(const StorageServerRequest *request, std::string type)
  {
    std::string r = request->row();
    std::string c = request->col();

    std::string v1;
    std::string v2;
    std::string value_type;
    if (request->has_is_bytes() && request->is_bytes())
    {
      v1 = request->value1_bytes();
      v2 = request->value2_bytes();
      value_type = "B";
    }
    else
    {
      v1 = request->value1();
      v2 = request->value2();
      value_type = "S";
    }
    std::string v1_size = std::to_string(v1.length());
    std::string v2_size = std::to_string(v2.length());

    std::string seq = std::to_string(request->sequence_number());

    std::string delim = " ";

    std::string serialized_string = seq + " ";
    if (type == "PUT")
    {
      serialized_string += "PUT" + delim + r + delim + c + delim + value_type + delim + v1_size + delim + v1;
    }
    else if (type == "CPUT")
    {
      serialized_string += "CPUT" + delim + r + delim + c + delim + value_type + delim + v1_size + delim + v2_size +
                           delim + v1 + delim + v2;
    }
    else
    {
      serialized_string += "DELETE" + delim + r + delim + c;
    }

    return std::to_string(serialized_string.length()) + " " + serialized_string;
  }

  void deserialize_and_process_command(std::string input)
  {
    std::stringstream ss(input);

    std::string seq;
    ss >> seq;

    std::string type;
    ss >> type;

    std::string row;
    ss >> row;

    std::string col;
    ss >> col;

    int tablet_index = get_tablet(row);
    std::string tablet_range = get_tablet_range(tablet_index);
    StorageServerRequest request;
    request.set_row(row);
    request.set_col(col);

    if (type == "PUT")
    {
      std::string value_type;
      ss >> value_type;

      std::string v1_size_str;
      ss >> v1_size_str;
      int v1_size = std::stoi(v1_size_str);
      std::string v1 = input.substr((seq + type + row + col + value_type + v1_size_str).length() + 6, v1_size);

      if (value_type == "B")
      {
        request.set_is_bytes(true);
        request.set_value1_bytes(v1);
      }
      else
      {
        request.set_is_bytes(false);
        request.set_value1(v1);
      }

      // std::cout << "CMD read from log: " << type << " " << row << " " << col << " " << v1 << std::endl;
      log_debug_with_tablet(tablet_range, "Applying log CMD: %s %s %s %s", type.c_str(), row.c_str(), col.c_str(),
                            v1.c_str());
      // apply command
      execute_put(&request, tablet_index);
    }
    else if (type == "CPUT")
    {
      std::string value_type;
      ss >> value_type;

      std::string v1_size_str;
      ss >> v1_size_str;
      int v1_size = std::stoi(v1_size_str);

      std::string v2_size_str;
      ss >> v2_size_str;
      int v2_size = std::stoi(v1_size_str);

      int text_length = seq.length() + type.length() + row.length() + col.length() + value_type.length() +
                        v1_size_str.length() + v2_size_str.length();
      std::string v1 = input.substr(text_length + 7, v1_size);
      std::string v2 = input.substr(text_length + v1_size + 7, v2_size);

      if (value_type == "B")
      {
        request.set_is_bytes(true);
        request.set_value1_bytes(v1);
        request.set_value2_bytes(v2);
      }
      else
      {
        request.set_is_bytes(false);
        request.set_value1(v1);
        request.set_value2(v2);
      }

      // std::cout << "CMD read from log: " << type << " " << row << " " << col << " " << v1 << " " << v2 << std::endl;
      log_debug_with_tablet(tablet_range, "Applying log CMD: %s %s %s %s %s", type.c_str(), row.c_str(), col.c_str(),
                            v1.c_str(), v2.c_str());
      execute_cput(&request, tablet_index);
    }
    else
    {
      request.set_is_bytes(false);

      // std::cout << "CMD read from log: " << type << " " << row << " " << col << std::endl;
      log_debug_with_tablet(tablet_range, "Applying log CMD: %s %s %s", type.c_str(), row.c_str(), col.c_str());
      execute_delete(&request, tablet_index);
    }

    return;
  }

  void update_curr_log_size(int tablet_index)
  {
    std::string tablet_range = get_tablet_range(tablet_index);
    std::string file_path = BACKUP_FILEPATH + "logs/" + tablet_range;

    if (std::filesystem::file_size(file_path) == 0)
    {
      return;
    }
    else
    {
      log_debug_with_tablet(tablet_range, "loading log");
    }
    std::shared_lock<std::shared_mutex> lock(log_locks[tablet_index]);

    std::ifstream infile(file_path);
    if (!infile)
    {
      std::cerr << "Failed to open log file" << std::endl;
    }

    bool is_first_cmd = true;
    int chars_rem = 0;
    std::string line;
    std::string command;
    int log_length = 0;
    while (std::getline(infile, line))
    {
      if (chars_rem <= 0)
      {
        if (!is_first_cmd)
        {
          log_length++;
        }
        is_first_cmd = false;

        int idx = line.find(' ');
        chars_rem = std::stoi(line.substr(0, idx));
        command = line.substr(idx + 1);
        chars_rem -= command.length();
      }
      else
      {
        command += "\n";
        command += line;
        chars_rem -= 1 + line.length();
      }
    }
    log_length++;

    tablet_log_size[tablet_index].first = log_length;
    tablet_log_size[tablet_index].second = std::filesystem::file_size(file_path);

    infile.close();
  }

  void load_log(int tablet_index)
  {
    std::string tablet_range = get_tablet_range(tablet_index);
    std::string log_path = BACKUP_FILEPATH + "logs/" + tablet_range;

    if (std::filesystem::file_size(log_path) == 0)
    {
      return;
    }
    else
    {
      log_debug_with_tablet(tablet_range, "loading log");
    }
    std::shared_lock<std::shared_mutex> lock(log_locks[tablet_index]);

    std::ifstream infile(log_path);
    if (!infile)
    {
      std::cerr << "Failed to open log file" << std::endl;
    }

    bool is_first_cmd = true;
    int chars_rem = 0;
    std::string line;
    std::string command;
    int log_length = 0;
    while (std::getline(infile, line))
    {
      // std::cout << line << std::endl;
      if (chars_rem <= 0)
      {
        // std::cout << "chars rem: " << chars_rem << std::endl;

        if (!is_first_cmd)
        {
          deserialize_and_process_command(command);
          log_length++;
        }
        is_first_cmd = false;

        int idx = line.find(' ');
        chars_rem = std::stoi(line.substr(0, idx));
        command = line.substr(idx + 1);
        chars_rem -= command.length();
      }
      else
      {
        command += "\n";
        command += line;
        chars_rem -= 1 + line.length();
      }
    }
    // process last command
    deserialize_and_process_command(command);
    log_length++;

    tablet_log_size[tablet_index].first = log_length;
    tablet_log_size[tablet_index].second = std::filesystem::file_size(log_path);

    infile.close();
  }

  void load_checkpoint(int tablet_index, int version)
  {
    std::string tablet_range = get_tablet_range(tablet_index);
    log_debug_with_tablet(tablet_range, "loading checkpoint: %d", version);

    // std::unordered_map<std::string, std::unordered_map<std::string, std::string>> kv_store;

    // clear old tablet
    // acquire exclusive lock on tablet

    tablet_to_kv_store[tablet_index].clear();

    std::string checkpoint_path = BACKUP_FILEPATH + "checkpoints/" + tablet_range + "/" + std::to_string(version);

    std::ifstream cp_file(checkpoint_path);
    if (!cp_file)
    {
      log_error_with_tablet(tablet_range, "failed to load checkpoint at path %s", checkpoint_path.c_str());
    }

    std::string line;
    std::string val;
    int chars_rem = 0;
    int cols_left = 0;
    int val_chars_left = 0;

    std::string row;
    std::string col;
    bool is_bytes = false;
    while (std::getline(cp_file, line))
    {
      if (val_chars_left > 0)
      {
        val += "\n";
        val += line;

        val_chars_left -= 1 + line.length();

        // value is multiline
        if (val_chars_left == 0)
        {
          // std::cout << "col: " << col << " val: " << val << std::endl;
          tablet_to_kv_store[tablet_index][row][col] = {val, is_bytes};
        }
      }
      else
      {
        std::stringstream ss(line);

        if (cols_left == 0)
        {
          ss >> row;

          std::string col_count_str;
          ss >> col_count_str;
          cols_left = std::stoi(col_count_str);
          // std::cout << "row: " << row << std::endl;
        }
        else
        {
          ss >> col;

          std::string is_bytes_str;
          ss >> is_bytes_str;
          is_bytes = is_bytes_str == "1" ? true : false;

          std::string val_size_str;
          ss >> val_size_str;
          val_chars_left = std::stoi(val_size_str);

          val = line.substr(col.length() + is_bytes_str.length() + val_size_str.length() + 3);
          val_chars_left -= val.length();
          cols_left--;

          // value fits in a single line
          if (val_chars_left == 0)
          {
            // std::cout << "col: " << col << " val: " << val << std::endl;
            tablet_to_kv_store[tablet_index][row][col] = {val, is_bytes};
          }
        }
      }
    }

    checkpoint_version[tablet_index] = version;
  }

  void create_checkpoint(int tablet_index)
  {
    int new_cp_version = checkpoint_version[tablet_index] + 1;

    std::string tablet_range = get_tablet_range(tablet_index);
    std::string checkpoint_path =
        BACKUP_FILEPATH + "checkpoints/" + tablet_range + "/" + std::to_string(new_cp_version);
    // std::cout << "path of checkpoint: " << checkpoint_path << std::endl;
    log_debug_with_tablet(tablet_range, "creating and saving checkpoint version %d", new_cp_version, " to disk");
    std::ofstream ofs(checkpoint_path, std::ios_base::app);
    if (!ofs)
    {
      log_error_with_tablet(tablet_range, "failed to craete checkpoint file");
      return;
    }
    for (auto &row_entry : tablet_to_kv_store[tablet_index])
    {
      std::string row = row_entry.first;
      int map_size = row_entry.second.size();
      // print: rowX col_count_X
      ofs << row << " " << map_size << std::endl;

      // print: colX is_bytes val_size_X valX
      for (auto &col_val : row_entry.second)
      {
        ofs << col_val.first << " " << col_val.second.is_bytes << " " << col_val.second.value.length() << " "
            << col_val.second.value << std::endl;
      }
    }
    ofs.close();

    checkpoint_version[tablet_index]++;
  }

  void handle_checkpointing(int tablet_index, std::string tablet_range, std::string file_path, int cmd_byte_count)
  {
    if (tablet_log_size[tablet_index].first + 1 == MAX_LOG_CMD_COUNT ||
        tablet_log_size[tablet_index].second + cmd_byte_count >= MAX_LOG_FILE_SIZE)
    {
      create_checkpoint(tablet_index);

      // empty out log
      log_debug_with_tablet(tablet_range, "clearing log");
      std::ofstream tablet_log(file_path, std::ios::trunc);
      if (!tablet_log.is_open())
      {
        log_debug_with_tablet(tablet_range, "failed to open log for clearing purposes");
        return;
      }
      tablet_log.close();

      tablet_log_size[tablet_index] = {0, 0};
    }
  }

  // return true iff log append was succesful
  bool append_to_tablet_log(const StorageServerRequest *request, std::string type)
  {
    std::string row = request->row();
    int tablet_index = get_tablet(row);
    if (tablet_index == -1)
      return false;

    std::string tablet_range = get_tablet_range(tablet_index);

    std::lock_guard log_lock(log_locks[tablet_index]);

    std::string serialized_command = serialize_command(request, type);

    std::string log_path = BACKUP_FILEPATH + "logs/" + tablet_range;

    // add 1 for the newline we're gonna add
    int cmd_byte_count = sizeof(serialized_command) + 1;

    handle_checkpointing(tablet_index, tablet_range, log_path, cmd_byte_count);

    log_debug_with_tablet(tablet_range, "logging command to be executed: %s", serialized_command.c_str());

    // append request to log
    std::ofstream ofs(log_path, std::ios_base::app);
    if (!ofs)
    {
      return false;
    }
    ofs << serialized_command << std::endl;
    ofs.close();

    tablet_log_size[tablet_index].first++;
    tablet_log_size[tablet_index].second = std::filesystem::file_size(log_path);

    return true;
  }

  Status handle_request(const StorageServerRequest *request, StorageServerResponse *response, std::string type)
  {
    std::string row = request->row();
    int tablet_index = get_tablet(row);

    if (tablet_index == -1)
    {
      log_error("requested row (%s) not managed by this server", row.c_str());
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "Requested row not managed by this server");
    }

    std::shared_lock<std::shared_mutex> lock(tablet_locks[tablet_index]);
    if (request->from_primary())
    {
      int num = request->sequence_number();

      {
        // acquire lock in a non-blocking manner
        std::unique_lock lock(locks[row]);
        // check if this request is the right one to execute based on ordering assigned by primary
        while (num > sequence_numbers[row] + 1)
        {
          // release lock and hibernate until told to check again
          cvs[row].wait(lock);
          // acquire lock in a non-blocking manner to check if this is the request to run next
        }
      }

      append_to_tablet_log(request, type);

      if (type == "PUT")
      {
        execute_put(request, tablet_index);
      }
      else if (type == "CPUT")
      {
        execute_cput(request, tablet_index);
      }
      else
      {
        execute_delete(request, tablet_index);
      }

      return Status::OK;
    }

    StorageServerRequest new_request;
    new_request.set_row(request->row());
    new_request.set_col(request->col());
    if (request->has_is_bytes())
      new_request.set_is_bytes(request->is_bytes());
    if (request->has_value1())
      new_request.set_value1(request->value1());
    if (request->has_value2())
      new_request.set_value2(request->value2());
    if (request->has_value1_bytes())
      new_request.set_value1_bytes(request->value1_bytes());
    if (request->has_value2_bytes())
      new_request.set_value2_bytes(request->value2_bytes());

    std::vector<std::string> ordered_replicas = get_ordered_replicas(tablet_index);
    std::string tablet_range = get_tablet_range(tablet_index);
    if (ordered_replicas[0] == my_address)
    {
      new_request.set_from_primary(true);

      {
        std::lock_guard<std::mutex> lock(locks[row]);
        new_request.set_sequence_number(++sequence_max_seen[row]);
      }

      log_debug_with_tablet(tablet_range, "forwarding user request to replicas as the primary");
      for (auto &address : ordered_replicas)
      {
        forward_request(address, new_request, type);
      }
    }
    else
    {
      log_debug_with_tablet(tablet_range, "forwarding user request to primary");
      while (1)
      {
        std::string current_primary = get_ordered_replicas(tablet_index)[0];
        // std::cout << "Current primary: " << current_primary << std::endl;
        if (!forward_request(current_primary, new_request, type))
        {
          // if current primary is down, pull new tablet information
          // std::cout << "DOWN" << std::endl;
          log_debug_with_tablet(tablet_range, "current primary (%s) is down", current_primary.c_str());
          get_tablets_metadata(current_primary, false);
        }
        else
        {
          break;
        }
      }
    }
    return Status::OK;
  }

  bool forward_request(const std::string &address, const StorageServerRequest &request, const std::string &type)
  {
    log_debug("forwarding request to replica->%s", address.c_str());
    StorageServerResponse response;
    ClientContext context;
    Status status;

    grpc::ChannelArguments channel_args;
    channel_args.SetMaxSendMessageSize(MAX_MESSAGE_SIZE);

    channel_args.SetMaxReceiveMessageSize(MAX_MESSAGE_SIZE);
    //

    std::unique_ptr<StorageServer::Stub> stub_(
        StorageServer::NewStub(grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), channel_args)));

    if (type == "PUT")
    {
      status = stub_->StoragePut(&context, request, &response);
    }
    else if (type == "CPUT")
    {
      status = stub_->StorageCPut(&context, request, &response);
    }
    else
    {
      status = stub_->StorageDelete(&context, request, &response);
    }

    return status.ok();
  }

  int get_tablet(const std::string &row)
  {
    std::shared_lock lock(tablet_metadata_lock);
    for (int i = 0; i < tablets.size(); ++i)
    {
      // if (strcmp(tablets[i].range.begin.c_str(), row.c_str()) <= 0 &&
      //     strcmp(tablets[i].range.end.c_str(), row.c_str()) > 0)
      // {
      //   return i;
      // }
      if (i == tablets.size() - 1)
      {
        if (tablets[i].range.begin <= row)
        {
          return i;
        }
      }
      else
      {
        if (tablets[i].range.begin <= row && row < tablets[i].range.end)
        {
          return i;
        }
      }
    }
    return -1;
  }

  std::string get_tablet_range(int tablet_index)
  {
    std::shared_lock lock(tablet_metadata_lock);
    return tablets[tablet_index].range.begin + "_" + tablets[tablet_index].range.end;
  }

  // initialize metadata for all tablets from scratch
  void fill_tablets(const PushStorageTabletsRequest *request)
  {
    {
      std::unique_lock lock(tablet_metadata_lock);
      tablets.clear();
      for (int i = 0; i < request->tablets_size(); ++i)
      {
        std::vector<ServerMetadata> replicas;
        std::string tablet_range = request->tablets(i).begin() + "_" + request->tablets(i).end();
        // std::cout << "[" << tablet_range << ") ";
        for (int j = 0; j < request->tablets(i).servers_size(); ++j)
        {
          replicas.push_back({request->tablets(i).servers(j).address(), request->tablets(i).servers(j).primary()});
          if (request->tablets(i).servers(j).primary())
          {
            // std::cout << request->tablets(i).servers(j).address();
          }
        }
        // std::cout << std::endl;

        Range range = {request->tablets(i).begin(), request->tablets(i).end()};
        tablets.push_back({range, replicas});
      }
    }
    for (auto &lock_pair : locks)
    {
      std::lock_guard row_lock(lock_pair.second);
      sequence_numbers[lock_pair.first] = 0;
      sequence_max_seen[lock_pair.first] = 0;
    }
  }

  std::vector<std::string> get_ordered_replicas(int index)
  {
    std::shared_lock lock(tablet_metadata_lock);
    std::vector<std::string> replicas;
    for (auto &metadata : tablets[index].replicas)
    {
      if (metadata.primary)
      {
        replicas.insert(replicas.begin(), metadata.address);
      }
      else
      {
        replicas.push_back(metadata.address);
      }
    }
    return replicas;
  }
};

void RunServer(const char *const port)
{
  std::string server_address("127.0.0.1:" + std::string(port));
  StorageServerImpl service{server_address};

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);

  builder.SetMaxSendMessageSize(1610612736);
  builder.SetMaxMessageSize(1610612736);
  builder.SetMaxReceiveMessageSize(1610612736);

  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  service.bootstrap_server();

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "Did not specify port" << std::endl;
    exit(EXIT_FAILURE);
  }
  RunServer(argv[argc - 1]);

  return 0;
}
