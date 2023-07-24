#!/bin/sh
set -ex
SCRIPT_DIR=$(dirname $(readlink -f "$0"))

$SCRIPT_DIR/buildPhase0.sh
$SCRIPT_DIR/buildFullPhase0.sh
$SCRIPT_DIR/buildPhase1.sh
$SCRIPT_DIR/buildFullPhase1.sh
$SCRIPT_DIR/buildPhase2.sh
$SCRIPT_DIR/buildFullPhase2.sh
