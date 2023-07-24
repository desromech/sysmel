#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$CC -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR/sysmel-pal.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
$CC -m32 -Wall -Wextra -Iinclude -I. -DNDEBUG $BUILD_SUPPORT_FLAGS -g -c -o$OUT_DIR32/sysmel-pal.o -O2 -ffunction-sections -fdata-sections lib/support/support.c
