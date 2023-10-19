#!/bin/sh
set -ex

. $(dirname $(readlink -f "$0"))/buildCommon.sh

$OUT_DIR/phase0 package-sources/EnvironmentPackages.sysmel -save-obj-file "$OUT_DIR/phase1.o"
$CC -o $OUT_DIR/phase1 $OUT_DIR/phase1.o $OUT_DIR/sysmel-pal.o $BUILD_LD_FLAGS
