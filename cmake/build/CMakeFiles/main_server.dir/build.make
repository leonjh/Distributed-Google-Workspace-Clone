# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/cis5050/.local/bin/cmake

# The command to remove a file.
RM = /home/cis5050/.local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cis5050/git/T07

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cis5050/git/T07/cmake/build

# Include any dependencies generated for this target.
include CMakeFiles/main_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/main_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/main_server.dir/flags.make

CMakeFiles/main_server.dir/src/frontend/main_server.cc.o: CMakeFiles/main_server.dir/flags.make
CMakeFiles/main_server.dir/src/frontend/main_server.cc.o: ../../src/frontend/main_server.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cis5050/git/T07/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/main_server.dir/src/frontend/main_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/main_server.dir/src/frontend/main_server.cc.o -c /home/cis5050/git/T07/src/frontend/main_server.cc

CMakeFiles/main_server.dir/src/frontend/main_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/main_server.dir/src/frontend/main_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cis5050/git/T07/src/frontend/main_server.cc > CMakeFiles/main_server.dir/src/frontend/main_server.cc.i

CMakeFiles/main_server.dir/src/frontend/main_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/main_server.dir/src/frontend/main_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cis5050/git/T07/src/frontend/main_server.cc -o CMakeFiles/main_server.dir/src/frontend/main_server.cc.s

# Object files for target main_server
main_server_OBJECTS = \
"CMakeFiles/main_server.dir/src/frontend/main_server.cc.o"

# External object files for target main_server
main_server_EXTERNAL_OBJECTS =

../../bin/main_server: CMakeFiles/main_server.dir/src/frontend/main_server.cc.o
../../bin/main_server: CMakeFiles/main_server.dir/build.make
../../bin/main_server: libss_grpc_proto.a
../../bin/main_server: libms_grpc_proto.a
../../bin/main_server: /home/cis5050/.local/lib/libgrpc++_reflection.a
../../bin/main_server: /home/cis5050/.local/lib/libgrpc++.a
../../bin/main_server: /home/cis5050/.local/lib/libprotobuf.a
../../bin/main_server: libms_grpc_proto.a
../../bin/main_server: /home/cis5050/.local/lib/libgrpc++_reflection.a
../../bin/main_server: /home/cis5050/.local/lib/libgrpc++.a
../../bin/main_server: /home/cis5050/.local/lib/libprotobuf.a
../../bin/main_server: /home/cis5050/.local/lib/libgrpc.a
../../bin/main_server: /home/cis5050/.local/lib/libcares.a
../../bin/main_server: /home/cis5050/.local/lib/libaddress_sorting.a
../../bin/main_server: /home/cis5050/.local/lib/libre2.a
../../bin/main_server: /home/cis5050/.local/lib/libupb.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_raw_hash_set.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_hashtablez_sampler.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_hash.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_city.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_low_level_hash.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_statusor.a
../../bin/main_server: /home/cis5050/.local/lib/libgpr.a
../../bin/main_server: /home/cis5050/.local/lib/libz.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_bad_variant_access.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_status.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_strerror.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_distributions.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_seed_sequences.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_pool_urbg.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_randen.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_randen_hwaes.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_randen_hwaes_impl.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_randen_slow.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_platform.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_internal_seed_material.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_random_seed_gen_exception.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_cord.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_bad_optional_access.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_cordz_info.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_cord_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_cordz_functions.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_exponential_biased.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_cordz_handle.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_crc_cord_state.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_crc32c.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_crc_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_crc_cpu_detect.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_str_format_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_synchronization.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_stacktrace.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_symbolize.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_debugging_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_demangle_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_graphcycles_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_malloc_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_time.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_strings.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_throw_delegate.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_int128.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_strings_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_base.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_spinlock_wait.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_raw_logging_internal.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_log_severity.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_civil_time.a
../../bin/main_server: /home/cis5050/.local/lib/libabsl_time_zone.a
../../bin/main_server: /home/cis5050/.local/lib/libssl.a
../../bin/main_server: /home/cis5050/.local/lib/libcrypto.a
../../bin/main_server: CMakeFiles/main_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cis5050/git/T07/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/main_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/main_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/main_server.dir/build: ../../bin/main_server

.PHONY : CMakeFiles/main_server.dir/build

CMakeFiles/main_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/main_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/main_server.dir/clean

CMakeFiles/main_server.dir/depend:
	cd /home/cis5050/git/T07/cmake/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cis5050/git/T07 /home/cis5050/git/T07 /home/cis5050/git/T07/cmake/build /home/cis5050/git/T07/cmake/build /home/cis5050/git/T07/cmake/build/CMakeFiles/main_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/main_server.dir/depend

