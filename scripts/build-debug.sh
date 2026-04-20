#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$ROOT_DIR/build/debug"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug "$ROOT_DIR"
cmake --build "$BUILD_DIR" --parallel