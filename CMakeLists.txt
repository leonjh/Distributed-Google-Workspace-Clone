cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_INIT "-lpthread -lresolv")
project(HelloWorld C CXX)

include(cmake/common.cmake)

#Include the Protocal Buffers Package
find_package(Protobuf REQUIRED)

# Set the output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../bin/")

# Set the source directories
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/storageservertest")
set(PROTO_DIR "${PROJECT_SOURCE_DIR}/src/lib/proto")

set(PROTO_SRCS "proto_srcs")
set(PROTO_HDRS "proto_hdrs")
set(GRPC_SRCS "grpc_srcs")
set(GRPC_HDRS "grpc_hdrs")

# Proto file
get_filename_component(ss_proto "./src/lib/proto/storageserver.proto" ABSOLUTE)
get_filename_component(ss_proto_path "${ss_proto}" PATH)

get_filename_component(ms_proto "./src/lib/proto/masterserver.proto" ABSOLUTE)
get_filename_component(ms_proto_path "${ms_proto}" PATH)

# Generated sources
set(ss_${PROTO_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/storageserver.pb.cc")
set(ss_${PROTO_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/storageserver.pb.h")
set(ss_${GRPC_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/storageserver.grpc.pb.cc")
set(ss_${GRPC_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/storageserver.grpc.pb.h")

set(ms_${PROTO_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/masterserver.pb.cc")
set(ms_${PROTO_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/masterserver.pb.h")
set(ms_${GRPC_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/masterserver.grpc.pb.cc")
set(ms_${GRPC_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/masterserver.grpc.pb.h")

# Storage Server Command
add_custom_command(
      OUTPUT "${ss_proto_srcs}" "${ss_proto_hdrs}" "${ss_grpc_srcs}" "${ss_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ss_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${ss_proto}"
      DEPENDS "${ss_proto}")

# Master Server Command
add_custom_command(
      OUTPUT "${ms_proto_srcs}" "${ms_proto_hdrs}" "${ms_grpc_srcs}" "${ms_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ms_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${ms_proto}"
      DEPENDS "${ms_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# ss_grpc_proto
add_library(ss_grpc_proto
  ${ss_grpc_srcs}
  ${ss_grpc_hdrs}
  ${ss_proto_srcs}
  ${ss_proto_hdrs})
target_link_libraries(ss_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# ms_grpc_proto
add_library(ms_grpc_proto
  ${ms_grpc_srcs}
  ${ms_grpc_hdrs}
  ${ms_proto_srcs}
  ${ms_proto_hdrs})
target_link_libraries(ms_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

set(TARGETS
    master_server
    master_server_test
)

set(TARGETS_TWO
    storage_server
    storage_server_test
    storage_server_test_automatic
)

# Targets greeter_[async_](client|server)
foreach(_target ${TARGETS})
  add_executable(${_target} "./src/backend/${_target}.cc")
  target_link_libraries(${_target}
    ss_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  target_link_libraries(${_target}
    ms_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

foreach(_target ${TARGETS_TWO})
  add_executable(${_target} "./src/backend/${_target}.cc")
  target_link_libraries(${_target}
    ss_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
  target_link_libraries(${_target}
    ms_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

# Targets greeter_[async_](client|server)
foreach(_target
  main_server  )
  add_executable(${_target} "./src/frontend/${_target}.cc")
  target_link_libraries(${_target}
  ss_grpc_proto
  ms_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    -lresolv
    -lutil
  )
  target_link_libraries(${_target}
    ms_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

# Targets greeter_[async_](client|server)
foreach(_target
  FrontendLoadBalancer  )
  add_executable(${_target} "./src/frontend/${_target}.cpp")
  target_link_libraries(${_target} pthread)
endforeach()

# Targets greeter_[async_](client|server)
foreach(_target
  smtp  )
  add_executable(${_target} "./src/frontend/webmail/${_target}.cc")
  target_link_libraries(${_target} 
    pthread
    ms_grpc_proto
    ss_grpc_proto
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    -lresolv
    -lutil
  )
endforeach()