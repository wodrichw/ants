# Find protobuf package  
find_package(Protobuf REQUIRED)

# Generate C++ files from .proto 
file(GLOB proto_files "*.proto")

# Generate the .pb.cc and .pb.h files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_files})

# Add the .pb.cc files to a library
add_library(ProtoSources ${PROTO_SRCS} ${PROTO_HDRS})

# Link the Protobuf library
target_link_libraries(ProtoSources ${PROTOBUF_LIBRARIES})

# Include the directory that the .pb.h files are generated into
target_include_directories(ProtoSources PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set(PROTOBUF_INCLUDE_DIRS_1 ${CMAKE_CURRENT_BINARY_DIR}/.. CACHE INTERNAL "PROTOBUF_INCLUDE_DIRS_1")