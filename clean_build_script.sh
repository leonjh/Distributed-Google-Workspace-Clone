#!/bin/bash

# clear backup logs on startup
./clear_logs.sh

# Create the build directory if it does not exist
mkdir -p cmake/build

# Change directory to the build directory
pushd cmake/build

# Run CMake with the specified CMAKE_PREFIX_PATH and the parent directory as the source directory
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..

# Build the project with 4 parallel jobs
make -j 4

# Return to the original directory
popd

