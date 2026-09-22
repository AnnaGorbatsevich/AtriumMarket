FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DUSERVER_FEATURE_TESTSUITE=OFF \
    && cmake --build build -j "$(nproc)" --target stats_service

FROM ubuntu:24.04

RUN apt-get update && apt-get install --no-install-recommends -y \
    libcurl4t64 \
    libboost-iostreams1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-program-options1.83.0 \
    libboost-stacktrace1.83.0 \
    libev4t64 \
    libjemalloc2 \
    libre2-10 \
    libfmt9 \
    libcctz2 \
    libyaml-cpp0.8 \
    libzstd1 \
    libcrypto++8 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /lib/libclickhouse-cpp-lib.so /lib/libclickhouse-cpp-lib.so
COPY --from=build /app/build/stats_service /app/stats_service
COPY configs /app/configs

EXPOSE 8050

CMD ["/app/stats_service", "--config", "/app/configs/static_config.yaml"]
