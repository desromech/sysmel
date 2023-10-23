#!/bin/sh

SCRIPT_DIR=$(dirname $(readlink -f "$0"))
TOP=$(dirname $(readlink -f "$SCRIPT_DIR/../"))

OUT_DIR="$TOP/build-bootstrap/linux/x64"
OUT_DIR32="$TOP/build-bootstrap/linux/x86"

CC="gcc"
SYSBVMI="$OUT_DIR/sysbvmi"
SYSBVMI32="$OUT_DIR32/sysbvmi"

BUILD_LD_FLAGS="-dl"

SDL2_CFLAGS=$(sdl2-config --cflags)
SDL2_LDFLAG=$(sdl2-config --libs)

if test -n "$SDL2_CFLAGS"; then
    BUILD_CFLAGS="-DUSE_SDL2 $SDL2_CFLAGS $BUILD_CFLAGS"
    BUILD_LD_FLAGS="$SDL2_LDFLAG $BUILD_LD_FLAGS"
fi

mkdir -p $OUT_DIR
mkdir -p $OUT_DIR32
