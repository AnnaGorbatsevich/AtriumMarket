FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j "$(nproc)" --target order_service

FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest

WORKDIR /app
COPY --from=build /app/build/order_service /app/order_service
COPY configs /app/configs

EXPOSE 8060

CMD ["/app/order_service", "--config", "/app/configs/static_config.yaml"]
