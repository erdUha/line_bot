#!/bin/bash
set -euo pipefail

if [ -f .env ]; then
  set -a
  source .env
  set +a
fi

cmake -B ./build -G Ninja \
    -DCMAKE_C_COMPILER=gcc-14 \
    -DCMAKE_CXX_COMPILER=g++-14 \
    -DCMAKE_CXX_STANDARD=20
cmake --build ./build --parallel

echo
./build/line_bot
