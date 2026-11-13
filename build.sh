#!/bin/bash
set -e

cmake -S . -B ./build \
    -DCMAKE_C_COMPILER=gcc-14 \
    -DCMAKE_CXX_COMPILER=g++-14 \
    -DCMAKE_CXX_STANDARD=20
cmake --build ./build -j $(nproc)

echo
./build/line_bot
