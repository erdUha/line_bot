FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-14 \
    g++-14 \
    cmake \
    ninja-build \
    pkg-config \
    libssl-dev \
    libpq-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14 \
    CXX=g++-14

WORKDIR /app

COPY . .

RUN --mount=type=secret,id=dotenv,target=/app/.env \
    --mount=type=cache,target=/app/build \
    bash -c 'set -a && source /app/.env && set +a && \
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel && \
    cp /app/build/line_bot /app/line_bot_bin'

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libpq5 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN useradd --system --create-home appuser
WORKDIR /app

COPY --from=builder /app/public /app/public
RUN chown -R appuser:appuser /app

COPY --chown=appuser:appuser --from=builder /app/line_bot_bin /app/line_bot

USER appuser
CMD ["/app/line_bot"]
