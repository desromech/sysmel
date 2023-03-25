#!/bin/sh
set -ex
OUT_DIR="build-fast"

mkdir -p $OUT_DIR
gcc -Wall -Wextra -Iinclude -I. -o $OUT_DIR/tuuvmi -g -O3 apps/interpreter/interpreter.c lib/tuuvm/unityBuild.c -lm

