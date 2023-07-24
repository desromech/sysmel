#!/bin/sh
set -ex
OUT_DIR="build-fast"

mkdir -p $OUT_DIR
gcc -Wall -Wextra -Iinclude -I. -DNDEBUG -o $OUT_DIR/sysbvmi -g -O3 apps/interpreter/interpreter.c lib/sysbvm/unityBuild.c -lm

