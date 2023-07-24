#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$SYSBVMI -nogc module-sources/Bootstrap/main.sysmel module-sources/CLI-Interpreter/main.sysmel -e "CLISysmelInterpreter buildNativeInterpreterImage saveTo: \"$OUT_DIR/phase0.o\""
$CC -o $OUT_DIR/phase0 $OUT_DIR/phase0.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
