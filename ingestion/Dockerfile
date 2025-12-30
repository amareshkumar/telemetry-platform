# Multi-stage build (smaller image, build cache)
FROM ubuntu:22.04 AS builder

# Install modern CMake (Ubuntu 22.04 has old CMake that doesn't support presets v6)
RUN apt-get update && \
    apt-get install -y build-essential ninja-build git wget && \
    wget -qO- "https://cmake.org/files/v3.28/cmake-3.28.1-linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
WORKDIR /src/ingestion
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
