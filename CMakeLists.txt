cmake_minimum_required(VERSION 3.16)
project(mini-kvstore
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Minimal in-memory key-value store"
)

# ───── Compiler settings ─────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Strict warning flags — as in Redis
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Werror
    -Wno-unused-parameter
    -Wshadow
    -Wformat=2
    -Wstrict-prototypes
)

# Clang-specific: suppress newline-at-eof warning
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-newline-eof)
endif()

# ───── Sanitizer support (Debug builds) ─────
option(ENABLE_ASAN  "Enable AddressSanitizer"           OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# ───── Library target ─────
add_library(kvstore_lib STATIC
    src/kvstore.c
)
target_include_directories(kvstore_lib PUBLIC src)

# ───── Executable ─────
add_executable(kvstore src/main.c)
target_link_libraries(kvstore PRIVATE kvstore_lib)

# ───── Tests ─────
enable_testing()

add_executable(test_kvstore tests/test_kvstore.c)
target_link_libraries(test_kvstore PRIVATE kvstore_lib)

add_test(
    NAME unit_tests
    COMMAND test_kvstore
)

# ───── Fix 3: Coverage instrumentation ─────
# Usage: cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
option(ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    foreach(tgt kvstore_lib kvstore test_kvstore)
        target_compile_options(${tgt} PRIVATE --coverage -O0 -g)
        target_link_options(${tgt} PRIVATE --coverage)
    endforeach()
endif()

# ───── Fix 10: libFuzzer target (requires Clang) ─────
# Usage: cmake -DENABLE_FUZZING=ON -DCMAKE_C_COMPILER=clang
option(ENABLE_FUZZING "Enable libFuzzer target" OFF)
if(ENABLE_FUZZING)
    add_executable(fuzz_kvstore tests/fuzz_kvstore.c)
    target_link_libraries(fuzz_kvstore PRIVATE kvstore_lib)
    target_compile_options(fuzz_kvstore PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_kvstore PRIVATE -fsanitize=fuzzer,address)
endif()

# ───── Install ─────
install(TARGETS kvstore DESTINATION bin)

# ───── CPack for packaging ─────
set(CPACK_PACKAGE_NAME "mini-kvstore")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
include(CPack)
