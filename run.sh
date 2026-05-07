#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$ROOT_DIR/output"
BINARY="$OUTPUT_DIR/wav_tp"
LOG_FILE="$OUTPUT_DIR/run.log"

mkdir -p "$OUTPUT_DIR"

echo "[build] Compilation du projet"

g++ -std=c++17 -O2 \
    "$ROOT_DIR/main.cpp" \
    "$ROOT_DIR/src/binary_utils.cpp" \
    "$ROOT_DIR/src/wav_file.cpp" \
    "$ROOT_DIR/src/audio_processing.cpp" \
    -I"$ROOT_DIR/include" \
    -o "$BINARY"

echo "[run] Execution du programme"

"$BINARY" >"$LOG_FILE" 2>&1

echo "Execution terminee."
echo "Binaire : $BINARY"
echo "Log     : $LOG_FILE"