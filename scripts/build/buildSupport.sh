#!/bin/sh
set -ex
. ./buildCommon.sh

gcc -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR/support-x64.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
gcc -m32 -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR32/support-x86.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
