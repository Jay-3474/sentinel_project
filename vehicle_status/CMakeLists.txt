cmake_minimum_required(VERSION 3.15)
project(SomeIpProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

# 1. Conan Find
find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(ZLIB REQUIRED)

# 2. External vsomeip path (Your pre-built directory)
set(EXTERNAL_VSOMEIP_DIR "/home/jay/vsomeip_build/vsomeip/install")
find_library(VSOMEIP_LIB NAMES vsomeip3 PATHS "${EXTERNAL_VSOMEIP_DIR}/lib" NO_DEFAULT_PATH)
find_path(VSOMEIP_INC NAMES vsomeip/vsomeip.hpp PATHS "${EXTERNAL_VSOMEIP_DIR}/include" NO_DEFAULT_PATH)

# 3. Proto Generation
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS auto_system.proto)

# 4. Binary targets
add_executable(server main_server.cpp ${PROTO_SRCS})
add_executable(client main_client.cpp ${PROTO_SRCS})

# Force output to /build/
set_target_properties(server client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_include_directories(server PRIVATE ${VSOMEIP_INC} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(server PRIVATE ${VSOMEIP_LIB} protobuf::libprotobuf Boost::boost ZLIB::ZLIB)

target_include_directories(client PRIVATE ${VSOMEIP_INC} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(client PRIVATE ${VSOMEIP_LIB} protobuf::libprotobuf Boost::boost ZLIB::ZLIB)
