#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
CXX="${CXX:-c++}"
CC="${CC:-cc}"

mkdir -p "$BUILD_DIR"
cd "$ROOT_DIR"

build_server() {
    "$CC" -O2 -c \
        "$ROOT_DIR/deps/SQLiteCpp/sqlite3/sqlite3.c" \
        -o "$BUILD_DIR/sqlite3.o"

    "$CXX" -std=c++20 -Wall -Wextra -Wpedantic \
        -I"$ROOT_DIR/deps/SQLiteCpp/include" \
        -I"$ROOT_DIR/deps/SQLiteCpp/sqlite3" \
        "$ROOT_DIR/server/main.cpp" \
        "$ROOT_DIR"/deps/SQLiteCpp/src/*.cpp \
        "$BUILD_DIR/sqlite3.o" \
        -pthread -ldl -lm \
        -o "$BUILD_DIR/server"
}

build_client() {
    local source="$ROOT_DIR/client/main.cpp"
    if [[ ! -f "$source" ]]; then
        printf 'Client source not found: %s\n' "$source" >&2
        return 2
    fi

    "$CXX" -std=c++20 -Wall -Wextra -Wpedantic \
        "$source" \
        -o "$BUILD_DIR/client"
}

case "${1:-}" in
    server)
        build_server
        exec "$BUILD_DIR/server"
        ;;
    client)
        build_client
        exec "$BUILD_DIR/client"
        ;;
    *)
        printf 'Usage: %s {client|server}\n' "${0##*/}" >&2
        exit 2
        ;;
esac