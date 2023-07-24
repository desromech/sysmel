#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$OUT_DIR/fullPhase0 -e "CLISysmelInterpreter buildNativeInterpreterImage saveTo: \"$OUT_DIR/phase1.o\""
$CC -o $OUT_DIR/phase1 $OUT_DIR/phase1.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
