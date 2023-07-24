#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$SYSBVMI -nogc module-sources/Bootstrap/main.sysmel module-sources/CLI-Interpreter/main.sysmel -e "CLISysmelInterpreter buildFullNativeInterpreterImage saveTo: \"$OUT_DIR/fullPhase0.o\""
$CC -o $OUT_DIR/fullPhase0 $OUT_DIR/fullPhase0.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
