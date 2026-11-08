# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-src")
  file(MAKE_DIRECTORY "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-src")
endif()
file(MAKE_DIRECTORY
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-build"
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix"
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/tmp"
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/src/libpqxx-populate-stamp"
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/src"
  "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/src/libpqxx-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/src/libpqxx-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/erduha/Desktop/line_bot/build/_deps/libpqxx-subbuild/libpqxx-populate-prefix/src/libpqxx-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
