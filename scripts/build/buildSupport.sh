#!/bin/sh
set -ex
OUT_DIR="build-support"
. ./buildCommon.sh

mkdir -p $OUT_DIR
gcc -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR/support-x86_64.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
gcc -m32 -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR/support-x86.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
