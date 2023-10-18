#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$OUT_DIR/phase1 -save-obj-file "$OUT_DIR/phase2.o"
$CC -o $OUT_DIR/phase2 $OUT_DIR/phase2.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
