#!/bin/sh

SCRIPT_DIR=$(dirname $(readlink -f "$0"))
TOP=$(dirname $(readlink -f "$SCRIPT_DIR/../"))

OUT_DIR="$TOP/build-bootstrap/linux/x64"
OUT_DIR32="$TOP/build-bootstrap/linux/x86"

CC="gcc"
SYSBVMI="$OUT_DIR/sysbvmi"
SYSBVMI32="$OUT_DIR32/sysbvmi"

mkdir -p $OUT_DIR
mkdir -p $OUT_DIR32
