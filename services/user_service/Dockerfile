FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j "$(nproc)" --target user_service

FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest

WORKDIR /app
COPY --from=build /app/build/user_service /app/user_service
COPY configs /app/configs

EXPOSE 8090

CMD ["/app/user_service", "--config", "/app/configs/static_config.yaml"]
