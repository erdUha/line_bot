cmake_minimum_required(VERSION 3.15)

project(shadows VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{DATABASE_URL})
    message(FATAL_ERROR "DATABASE_URL is not set")
endif()

set(DATABASE_URL "$ENV{DATABASE_URL}")

if(NOT DEFINED ENV{ADMIN_TG_ID})
    message(FATAL_ERROR "ADMIN_TG_ID is not set")
endif()

set(ADMIN_TG_ID "$ENV{ADMIN_TG_ID}")

if(NOT DEFINED ENV{ADMIN_USERNAME})
    message(FATAL_ERROR "ADMIN_USERNAME is not set")
endif()

set(ADMIN_USERNAME "$ENV{ADMIN_USERNAME}")

if(NOT DEFINED ENV{GROUP_PASSWORD})
    message(FATAL_ERROR "GROUP_PASSWORD is not set")
endif()

set(GROUP_PASSWORD "$ENV{GROUP_PASSWORD}")


configure_file(
    "${CMAKE_SOURCE_DIR}/include/config.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/config.hpp"
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
  crypt
)
