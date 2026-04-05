# mini-kvstore

Minimal in-memory key-value store written in C, built with CMake, and shipped with a production-grade CI/CD pipeline.

[![CI Pipeline](https://github.com/solardome/mini-kvstore/actions/workflows/ci.yml/badge.svg)](https://github.com/solardome/mini-kvstore/actions/workflows/ci.yml)
![Version](https://img.shields.io/github/v/release/solardome/mini-kvstore)

## What is this

A simple hash table implementation (DJB2 hashing, open addressing) exposed through a clean C API: `set`, `get`, `del`, `count`. The project exists primarily as a reference implementation for building robust CI/CD pipelines around C projects using CMake and GitHub Actions.

## Motivation

Redis is written in C. C projects have unique CI/CD challenges - multiple compilers, sanitizers, memory safety tools, and build system complexity that Go or Java pipelines don't face. This project explores what a production-grade CI/CD pipeline looks like for a C codebase: layered static analysis, runtime sanitizers, fuzzing, container scanning, and a deterministic release gate - all integrated into GitHub Actions.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(nproc)
```

## Run

```bash
./build/kvstore
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Build with sanitizers

AddressSanitizer (memory errors, leaks):

```bash
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-asan --parallel $(nproc)
ctest --test-dir build-asan
```

UndefinedBehaviorSanitizer:

```bash
cmake -B build-ubsan -DCMAKE_BUILD_TYPE=Debug -DENABLE_UBSAN=ON
cmake --build build-ubsan --parallel $(nproc)
ctest --test-dir build-ubsan
```

## Docker

```bash
docker build -t mini-kvstore .
docker run --rm mini-kvstore
```

The Dockerfile uses a multi-stage build: compilation and testing happen in the builder stage, and the final image contains only the static binary running as a non-root user.

## CI Pipeline

The GitHub Actions pipeline (`.github/workflows/ci.yml`) runs on every push and PR:

| Stage | What it does |
|-------|-------------|
| **Build & Test** | Compiles with both `gcc` and `clang`, runs unit tests |
| **Sanitizers** | Runs tests under ASan and UBSan (Debug build) |
| **Lint** | `clang-tidy`, `cppcheck`, banned functions check |
| **Security** | `flawfinder` SAST scan for C-specific vulnerabilities |
| **Docker** | Multi-stage Docker build + Trivy container scan |
| **Release Gate** | Deterministic release decision on `main` branch |

## API

```c
kvs_store_t *kvs_create(size_t initial_capacity);
void         kvs_destroy(kvs_store_t *store);
int          kvs_set(kvs_store_t *store, const char *key, const char *value);
const char  *kvs_get(kvs_store_t *store, const char *key);
int          kvs_del(kvs_store_t *store, const char *key);
size_t       kvs_count(kvs_store_t *store);
```

Returns: `KVS_OK` (0), `KVS_ERR` (-1), or `KVS_NOT_FOUND` (-2).

## Project structure

```
mini-kvstore/
├── src/
│   ├── kvstore.h          # Public API
│   ├── kvstore.c          # Hash table implementation
│   └── main.c             # Demo binary
├── tests/
│   └── test_kvstore.c     # Unit tests
├── scripts/
│   ├── check-banned-functions.sh
│   └── release-gate.sh
├── CMakeLists.txt
├── Dockerfile
├── .clang-tidy
├── .clang-format
└── .github/workflows/
    └── ci.yml
```

## License

Apache License 2.0
