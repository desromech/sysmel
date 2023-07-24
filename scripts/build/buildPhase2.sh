#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$OUT_DIR/fullPhase1 -e "CLISysmelInterpreter buildNativeInterpreterImage saveTo: \"$OUT_DIR/phase2.o\""
$CC -o $OUT_DIR/phase2 $OUT_DIR/phase2.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
