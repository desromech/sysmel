#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

mkdir -p $OUT_DIR
$CC -Wall -Wextra -Iinclude -I. -DNDEBUG -o $OUT_DIR/sysbvmi -g -O3 apps/interpreter/interpreter.c lib/sysbvm/unityBuild.c -lm
$CC -m32 -Wall -Wextra -Iinclude -I. -DNDEBUG -o $OUT_DIR32/sysbvmi -g -O3 apps/interpreter/interpreter.c lib/sysbvm/unityBuild.c -lm
