// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: masterserver.proto

#include "masterserver.pb.h"
#include "masterserver.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace backend {

static const char* MasterServer_method_names[] = {
  "/backend.MasterServer/GetTabletsForUser",
  "/backend.MasterServer/GetTabletsForServer",
  "/backend.MasterServer/GetKVStoreState",
  "/backend.MasterServer/KillStorageServer",
  "/backend.MasterServer/ReviveStorageServer",
  "/backend.MasterServer/IsInitialStorageServerStartup",
  "/backend.MasterServer/GetStorageServerStatuses",
};

std::unique_ptr< MasterServer::Stub> MasterServer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< MasterServer::Stub> stub(new MasterServer::Stub(channel, options));
  return stub;
}

MasterServer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_GetTabletsForUser_(MasterServer_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetTabletsForServer_(MasterServer_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetKVStoreState_(MasterServer_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_KillStorageServer_(MasterServer_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_ReviveStorageServer_(MasterServer_method_names[4], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_IsInitialStorageServerStartup_(MasterServer_method_names[5], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetStorageServerStatuses_(MasterServer_method_names[6], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status MasterServer::Stub::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::backend::GetTabletsUserResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetTabletsUserRequest, ::backend::GetTabletsUserResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetTabletsForUser_, context, request, response);
}

void MasterServer::Stub::async::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsUserResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetTabletsUserRequest, ::backend::GetTabletsUserResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForUser_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsUserResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForUser_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsUserResponse>* MasterServer::Stub::PrepareAsyncGetTabletsForUserRaw(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::GetTabletsUserResponse, ::backend::GetTabletsUserRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetTabletsForUser_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsUserResponse>* MasterServer::Stub::AsyncGetTabletsForUserRaw(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetTabletsForUserRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::backend::MasterEmpty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetTabletsServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetTabletsForServer_, context, request, response);
}

void MasterServer::Stub::async::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::MasterEmpty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetTabletsServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForServer_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::MasterEmpty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::PrepareAsyncGetTabletsForServerRaw(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::MasterEmpty, ::backend::GetTabletsServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetTabletsForServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::AsyncGetTabletsForServerRaw(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetTabletsForServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::GetKVStoreState(::grpc::ClientContext* context, const ::backend::MasterEmpty& request, ::backend::MasterGetKVStoreStateResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::MasterEmpty, ::backend::MasterGetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetKVStoreState_, context, request, response);
}

void MasterServer::Stub::async::GetKVStoreState(::grpc::ClientContext* context, const ::backend::MasterEmpty* request, ::backend::MasterGetKVStoreStateResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::MasterEmpty, ::backend::MasterGetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVStoreState_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetKVStoreState(::grpc::ClientContext* context, const ::backend::MasterEmpty* request, ::backend::MasterGetKVStoreStateResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVStoreState_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterGetKVStoreStateResponse>* MasterServer::Stub::PrepareAsyncGetKVStoreStateRaw(::grpc::ClientContext* context, const ::backend::MasterEmpty& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::MasterGetKVStoreStateResponse, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetKVStoreState_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterGetKVStoreStateResponse>* MasterServer::Stub::AsyncGetKVStoreStateRaw(::grpc::ClientContext* context, const ::backend::MasterEmpty& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetKVStoreStateRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::KillStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::backend::MasterEmpty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_KillStorageServer_, context, request, response);
}

void MasterServer::Stub::async::KillStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_KillStorageServer_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::KillStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_KillStorageServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::PrepareAsyncKillStorageServerRaw(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::MasterEmpty, ::backend::KillStorageServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_KillStorageServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::AsyncKillStorageServerRaw(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncKillStorageServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::ReviveStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::backend::MasterEmpty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_ReviveStorageServer_, context, request, response);
}

void MasterServer::Stub::async::ReviveStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_ReviveStorageServer_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::ReviveStorageServer(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_ReviveStorageServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::PrepareAsyncReviveStorageServerRaw(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::MasterEmpty, ::backend::KillStorageServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_ReviveStorageServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::MasterEmpty>* MasterServer::Stub::AsyncReviveStorageServerRaw(::grpc::ClientContext* context, const ::backend::KillStorageServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncReviveStorageServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::IsInitialStorageServerStartup(::grpc::ClientContext* context, const ::backend::IsInitialStorageServerStartupRequest& request, ::backend::IsInitialStorageServerStartupResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::IsInitialStorageServerStartupRequest, ::backend::IsInitialStorageServerStartupResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_IsInitialStorageServerStartup_, context, request, response);
}

void MasterServer::Stub::async::IsInitialStorageServerStartup(::grpc::ClientContext* context, const ::backend::IsInitialStorageServerStartupRequest* request, ::backend::IsInitialStorageServerStartupResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::IsInitialStorageServerStartupRequest, ::backend::IsInitialStorageServerStartupResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_IsInitialStorageServerStartup_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::IsInitialStorageServerStartup(::grpc::ClientContext* context, const ::backend::IsInitialStorageServerStartupRequest* request, ::backend::IsInitialStorageServerStartupResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_IsInitialStorageServerStartup_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::IsInitialStorageServerStartupResponse>* MasterServer::Stub::PrepareAsyncIsInitialStorageServerStartupRaw(::grpc::ClientContext* context, const ::backend::IsInitialStorageServerStartupRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::IsInitialStorageServerStartupResponse, ::backend::IsInitialStorageServerStartupRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_IsInitialStorageServerStartup_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::IsInitialStorageServerStartupResponse>* MasterServer::Stub::AsyncIsInitialStorageServerStartupRaw(::grpc::ClientContext* context, const ::backend::IsInitialStorageServerStartupRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncIsInitialStorageServerStartupRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::GetStorageServerStatuses(::grpc::ClientContext* context, const ::backend::GetStorageServerStatusesRequest& request, ::backend::GetStorageServerStatusesResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetStorageServerStatusesRequest, ::backend::GetStorageServerStatusesResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetStorageServerStatuses_, context, request, response);
}

void MasterServer::Stub::async::GetStorageServerStatuses(::grpc::ClientContext* context, const ::backend::GetStorageServerStatusesRequest* request, ::backend::GetStorageServerStatusesResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetStorageServerStatusesRequest, ::backend::GetStorageServerStatusesResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetStorageServerStatuses_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetStorageServerStatuses(::grpc::ClientContext* context, const ::backend::GetStorageServerStatusesRequest* request, ::backend::GetStorageServerStatusesResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetStorageServerStatuses_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::GetStorageServerStatusesResponse>* MasterServer::Stub::PrepareAsyncGetStorageServerStatusesRaw(::grpc::ClientContext* context, const ::backend::GetStorageServerStatusesRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::GetStorageServerStatusesResponse, ::backend::GetStorageServerStatusesRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetStorageServerStatuses_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::GetStorageServerStatusesResponse>* MasterServer::Stub::AsyncGetStorageServerStatusesRaw(::grpc::ClientContext* context, const ::backend::GetStorageServerStatusesRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetStorageServerStatusesRaw(context, request, cq);
  result->StartCall();
  return result;
}

MasterServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetTabletsUserRequest, ::backend::GetTabletsUserResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetTabletsUserRequest* req,
             ::backend::GetTabletsUserResponse* resp) {
               return service->GetTabletsForUser(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetTabletsServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetTabletsServerRequest* req,
             ::backend::MasterEmpty* resp) {
               return service->GetTabletsForServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::MasterEmpty, ::backend::MasterGetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::MasterEmpty* req,
             ::backend::MasterGetKVStoreStateResponse* resp) {
               return service->GetKVStoreState(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::KillStorageServerRequest* req,
             ::backend::MasterEmpty* resp) {
               return service->KillStorageServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[4],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::KillStorageServerRequest, ::backend::MasterEmpty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::KillStorageServerRequest* req,
             ::backend::MasterEmpty* resp) {
               return service->ReviveStorageServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[5],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::IsInitialStorageServerStartupRequest, ::backend::IsInitialStorageServerStartupResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::IsInitialStorageServerStartupRequest* req,
             ::backend::IsInitialStorageServerStartupResponse* resp) {
               return service->IsInitialStorageServerStartup(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[6],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetStorageServerStatusesRequest, ::backend::GetStorageServerStatusesResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetStorageServerStatusesRequest* req,
             ::backend::GetStorageServerStatusesResponse* resp) {
               return service->GetStorageServerStatuses(ctx, req, resp);
             }, this)));
}

MasterServer::Service::~Service() {
}

::grpc::Status MasterServer::Service::GetTabletsForUser(::grpc::ServerContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsUserResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::GetTabletsForServer(::grpc::ServerContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::MasterEmpty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::GetKVStoreState(::grpc::ServerContext* context, const ::backend::MasterEmpty* request, ::backend::MasterGetKVStoreStateResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::KillStorageServer(::grpc::ServerContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::ReviveStorageServer(::grpc::ServerContext* context, const ::backend::KillStorageServerRequest* request, ::backend::MasterEmpty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::IsInitialStorageServerStartup(::grpc::ServerContext* context, const ::backend::IsInitialStorageServerStartupRequest* request, ::backend::IsInitialStorageServerStartupResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::GetStorageServerStatuses(::grpc::ServerContext* context, const ::backend::GetStorageServerStatusesRequest* request, ::backend::GetStorageServerStatusesResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace backend

