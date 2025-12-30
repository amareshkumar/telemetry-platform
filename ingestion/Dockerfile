# Multi-stage build (smaller image, build cache)
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y build-essential cmake ninja-build git
WORKDIR /src
COPY . .
RUN cmake --preset linux-ninja-release && \
    cmake --build --preset linux-ninja-release --target gateway_app

FROM ubuntu:22.04 AS runtime
LABEL org.opencontainers.image.source=https://github.com/amareshkumar/telemetry-platform
LABEL org.opencontainers.image.description="TelemetryHub Gateway - High-Performance C++ Telemetry Ingestion"
LABEL org.opencontainers.image.licenses=MIT
LABEL org.opencontainers.image.version="5.0.0"
RUN apt-get update && apt-get install -y libstdc++6 curl && rm -rf /var/lib/apt/lists/*
COPY --from=builder /src/ingestion/build/gateway/gateway_app /usr/local/bin/
COPY ingestion/docs/config.example.ini /etc/telemetryhub/config.ini
EXPOSE 8080
HEALTHCHECK --interval=30s CMD curl -f http://localhost:8080/health || exit 1
CMD ["gateway_app", "--config", "/etc/telemetryhub/config.ini"]
