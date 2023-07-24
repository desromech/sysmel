#!/bin/sh
set -ex
SCRIPT_DIR=$(dirname $(readlink -f "$0"))

$SCRIPT_DIR/buildBootstrapInterpreter.sh
$SCRIPT_DIR/buildSupport.sh
$SCRIPT_DIR/buildAllPhases.sh
