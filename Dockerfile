# Fix 5: upgraded to ubuntu:24.04 (GCC 13, glibc 2.39, longer support runway)
# Pin to a specific digest with: docker pull ubuntu:24.04 && docker inspect ubuntu:24.04 --format '{{index .RepoDigests 0}}'
# Renovate/Dependabot will keep this digest current automatically.

# ── Stage 1: Build ──
FROM ubuntu:24.04 AS builder

# OCI standard labels
LABEL org.opencontainers.image.title="mini-kvstore"
LABEL org.opencontainers.image.description="Minimal in-memory key-value store"
LABEL org.opencontainers.image.licenses="MIT"

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake -B out -DCMAKE_BUILD_TYPE=Release \
    && cmake --build out --parallel "$(nproc)" \
    && ctest --test-dir out --output-on-failure

# ── Stage 2: Runtime ──
FROM ubuntu:24.04

# Upgrade all packages to pick up security patches (e.g. CVE-2026-29111)
RUN apt-get update && apt-get upgrade -y --no-install-recommends \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -r kvstore && useradd -r -g kvstore kvstore

COPY --from=builder /build/out/kvstore /usr/local/bin/kvstore

USER kvstore
ENTRYPOINT ["kvstore"]
