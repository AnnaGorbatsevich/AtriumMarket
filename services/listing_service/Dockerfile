FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS build

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j "$(nproc)" --target listing_service

FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest

WORKDIR /app
COPY --from=build /app/build/listing_service /app/listing_service
COPY configs /app/configs

EXPOSE 8070

CMD ["/app/listing_service", "--config", "/app/configs/static_config.yaml"]
