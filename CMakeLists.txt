cmake_minimum_required(VERSION 3.15)

project(shadows VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(line_bot ${SOURCES})

add_subdirectory(deps/libpqxx build-pqxx)

target_include_directories(line_bot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(line_bot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/cpp-httplib")
target_include_directories(line_bot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/json/include")

target_link_libraries(line_bot PRIVATE
  pqxx
  OpenSSL::Crypto
)
