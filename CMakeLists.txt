cmake_minimum_required(VERSION 3.15)

project(shadows VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{DATABASE_URL})
    message(FATAL_ERROR "DATABASE_URL is not set")
endif()

set(DATABASE_URL "$ENV{DATABASE_URL}")

configure_file(
    "${CMAKE_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_BINARY_DIR}/generated/config.h"
    @ONLY
)

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(line_bot ${SOURCES})

add_subdirectory(deps/libpqxx build-pqxx)
add_subdirectory(deps/json build-json)
add_subdirectory(deps/cpp-httplib build-httplib)

target_include_directories(line_bot PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_BINARY_DIR}/generated"
  )

target_link_libraries(line_bot PRIVATE
  pqxx
  nlohmann_json::nlohmann_json
  httplib::httplib
  OpenSSL::Crypto
)
