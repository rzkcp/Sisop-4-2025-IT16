FROM debian:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    libfuse-dev \
    fuse \
    inotify-tools \
    --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY fuse_operations.c .

RUN gcc fuse_operations.c -o antink_fuse `pkg-config fuse --cflags --libs`

RUN mkdir -p /it24_host /antink_mount /antink_logs /var/log

ENTRYPOINT ["/app/antink_fuse"]
