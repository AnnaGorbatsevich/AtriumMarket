FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j "$(nproc)" --target gateway

FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest

WORKDIR /app
COPY --from=build /app/build/gateway /app/gateway
COPY configs /app/configs

EXPOSE 8080

CMD ["/app/gateway", "--config", "/app/configs/static_config.yaml"]
