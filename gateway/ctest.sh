#!/usr/bin/env sh
# Thin wrapper so running gateway's tests looks close to plain `ctest`.
# Usage (from gateway/): ./ctest.sh -V
#
# Rebuilds the gateway binary (fast, ccache-backed, no-op if nothing
# changed) and then forwards all arguments to the real `ctest` running
# inside the userver dev image.
set -e

export MSYS_NO_PATHCONV=1
cd "$(dirname "$0")"

if [ ! -f build/CMakeCache.txt ]; then
    docker run --rm --user 1000:1000 -e HOME=/tmp \
        -v "$(pwd):/app" -w /app \
        ghcr.io/userver-framework/ubuntu-24.04-userver:latest \
        cmake -B build -DCMAKE_BUILD_TYPE=Release
fi

docker run --rm --user 1000:1000 -e HOME=/tmp \
    -v "$(pwd):/app" -w /app \
    ghcr.io/userver-framework/ubuntu-24.04-userver:latest \
    sh -c "cmake --build build -j\$(nproc) --target gateway && ctest --test-dir build $*"
