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
  {}

::grpc::Status MasterServer::Stub::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::backend::GetTabletsResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetTabletsUserRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetTabletsForUser_, context, request, response);
}

void MasterServer::Stub::async::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetTabletsUserRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForUser_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetTabletsForUser(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForUser_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsResponse>* MasterServer::Stub::PrepareAsyncGetTabletsForUserRaw(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::GetTabletsResponse, ::backend::GetTabletsUserRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetTabletsForUser_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsResponse>* MasterServer::Stub::AsyncGetTabletsForUserRaw(::grpc::ClientContext* context, const ::backend::GetTabletsUserRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetTabletsForUserRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::backend::GetTabletsResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetTabletsServerRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetTabletsForServer_, context, request, response);
}

void MasterServer::Stub::async::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::GetTabletsResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetTabletsServerRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForServer_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetTabletsForServer(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::GetTabletsResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTabletsForServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsResponse>* MasterServer::Stub::PrepareAsyncGetTabletsForServerRaw(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::GetTabletsResponse, ::backend::GetTabletsServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetTabletsForServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::GetTabletsResponse>* MasterServer::Stub::AsyncGetTabletsForServerRaw(::grpc::ClientContext* context, const ::backend::GetTabletsServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetTabletsForServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status MasterServer::Stub::GetKVStoreState(::grpc::ClientContext* context, const ::backend::GetKVStoreStateRequest& request, ::backend::GetKVStoreStateResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::backend::GetKVStoreStateRequest, ::backend::GetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetKVStoreState_, context, request, response);
}

void MasterServer::Stub::async::GetKVStoreState(::grpc::ClientContext* context, const ::backend::GetKVStoreStateRequest* request, ::backend::GetKVStoreStateResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::backend::GetKVStoreStateRequest, ::backend::GetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVStoreState_, context, request, response, std::move(f));
}

void MasterServer::Stub::async::GetKVStoreState(::grpc::ClientContext* context, const ::backend::GetKVStoreStateRequest* request, ::backend::GetKVStoreStateResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetKVStoreState_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::backend::GetKVStoreStateResponse>* MasterServer::Stub::PrepareAsyncGetKVStoreStateRaw(::grpc::ClientContext* context, const ::backend::GetKVStoreStateRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::backend::GetKVStoreStateResponse, ::backend::GetKVStoreStateRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetKVStoreState_, context, request);
}

::grpc::ClientAsyncResponseReader< ::backend::GetKVStoreStateResponse>* MasterServer::Stub::AsyncGetKVStoreStateRaw(::grpc::ClientContext* context, const ::backend::GetKVStoreStateRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetKVStoreStateRaw(context, request, cq);
  result->StartCall();
  return result;
}

MasterServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetTabletsUserRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetTabletsUserRequest* req,
             ::backend::GetTabletsResponse* resp) {
               return service->GetTabletsForUser(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetTabletsServerRequest, ::backend::GetTabletsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetTabletsServerRequest* req,
             ::backend::GetTabletsResponse* resp) {
               return service->GetTabletsForServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MasterServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MasterServer::Service, ::backend::GetKVStoreStateRequest, ::backend::GetKVStoreStateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MasterServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::backend::GetKVStoreStateRequest* req,
             ::backend::GetKVStoreStateResponse* resp) {
               return service->GetKVStoreState(ctx, req, resp);
             }, this)));
}

MasterServer::Service::~Service() {
}

::grpc::Status MasterServer::Service::GetTabletsForUser(::grpc::ServerContext* context, const ::backend::GetTabletsUserRequest* request, ::backend::GetTabletsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::GetTabletsForServer(::grpc::ServerContext* context, const ::backend::GetTabletsServerRequest* request, ::backend::GetTabletsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status MasterServer::Service::GetKVStoreState(::grpc::ServerContext* context, const ::backend::GetKVStoreStateRequest* request, ::backend::GetKVStoreStateResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace backend
