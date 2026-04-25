cmake_minimum_required(VERSION 3.15)
project(SentinelProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(ZLIB REQUIRED)

set(EXTERNAL_VSOMEIP_DIR "/home/jay/vsomeip_build/vsomeip/install")
find_library(VSOMEIP_LIB NAMES vsomeip3 PATHS "${EXTERNAL_VSOMEIP_DIR}/lib" NO_DEFAULT_PATH)
find_path(VSOMEIP_INC NAMES vsomeip/vsomeip.hpp PATHS "${EXTERNAL_VSOMEIP_DIR}/include" NO_DEFAULT_PATH)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS auto_system.proto)

add_executable(gateway_server src/server/main_server.cpp ${PROTO_SRCS})
add_executable(telemetry_client src/clients/telemetry_client.cpp ${PROTO_SRCS})
add_executable(shell_client src/clients/shell_client.cpp ${PROTO_SRCS})

foreach(target gateway_server telemetry_client shell_client)
    target_include_directories(${target} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_BINARY_DIR}
        ${VSOMEIP_INC}
    )
    target_link_libraries(${target} PRIVATE 
        ${VSOMEIP_LIB} 
        protobuf::libprotobuf 
        Boost::boost 
        ZLIB::ZLIB
    )
endforeach()
