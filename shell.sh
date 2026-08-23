#!/usr/bin/env sh
# Opens an interactive shell inside the userver dev container, with the
# whole repo mounted at /app and configured as one CMake build tree (the
# root CMakeLists.txt aggregates every service via add_subdirectory()).
# From inside:
#
#   cmake --build build -j$(nproc)                         # build everything
#   ctest --test-dir build --output-on-failure             # run every service's tests
#   ctest --test-dir build -R gateway --output-on-failure  # just gateway's tests
#   build/gateway/runtests-gateway -k test_ping -v --no-header -p no:cacheprovider --color=yes
#                                                            # one test, readable output
#
# `exit` to leave - the container was started with --rm, so it's cleaned
# up automatically.
#
# Usage (from the repo root): bash shell.sh
set -e

export MSYS_NO_PATHCONV=1
cd "$(dirname "$0")"

IMAGE=ghcr.io/userver-framework/ubuntu-24.04-userver:latest

if [ ! -f build/CMakeCache.txt ]; then
    docker run --rm --user 1000:1000 -e HOME=/tmp \
        -v "$(pwd):/app" -w /app \
        "$IMAGE" cmake -B build -DCMAKE_BUILD_TYPE=Release
fi

docker run --rm -it --user 1000:1000 -e HOME=/tmp \
    -v "$(pwd):/app" -w /app \
    "$IMAGE" bash
