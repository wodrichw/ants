message(STATUS "Compiling Protobuf messages")
# Find protobuf package  
find_package(Protobuf REQUIRED)

# Generate C++ files from .proto 
file(GLOB proto_files "*.proto")

# Generate the .pb.cc and .pb.h files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_files})

# Add the .pb.cc files to a library
add_library(ProtoSources ${PROTO_SRCS} ${PROTO_HDRS})

# Link the Protobuf library and its absl dependencies
find_package(absl CONFIG REQUIRED)
target_link_libraries(ProtoSources PUBLIC
	protobuf::libprotobuf
	absl::absl_log
	absl::absl_check
	absl::strings
	absl::status
	absl::statusor
	absl::hash
	absl::raw_hash_set
	absl::raw_hash_map
	absl::flat_hash_set
	absl::flat_hash_map
	absl::synchronization
	absl::base
	absl::str_format
	absl::str_format_internal
	absl::strings_internal
	absl::raw_logging_internal
	absl::hashtablez_sampler
	absl::hashtable_debug
	absl::spinlock_wait
	absl::throw_delegate
	absl::log_severity
	absl::log_internal_check_op
	absl::log_internal_message
	absl::log_internal_nullguard
	absl::log_internal_proto
	absl::log_internal_format
	absl::log_internal_globals
	absl::log_internal_check_impl
	absl::log_internal_conditions
	absl::log_internal_log_impl
	absl::log_internal_log_sink_set
	absl::log_internal_structured_proto
)

# Include the directory that the .pb.h files are generated into
target_include_directories(ProtoSources PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
)

set(PROTOBUF_INCLUDE_DIRS_1 "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/.." CACHE INTERNAL "PROTOBUF_INCLUDE_DIRS_1")