# Find protobuf package  
find_package(Protobuf REQUIRED)  
# Generate C++ files from .proto 
file(GLOB proto_srcs "*.proto")
add_library(ProtoSources ${proto_srcs})
protobuf_generate(TARGET ProtoSources
  LANGUAGE cpp
)

set(PROTOBUF_INCLUDE_DIRS_1   ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "PROTOBUF_INCLUDE_DIRS_1")
