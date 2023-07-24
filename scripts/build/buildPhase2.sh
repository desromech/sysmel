#!/bin/sh
set -ex

. ./buildCommon.sh

./fullPhase1 -e 'CLISysmelInterpreter buildNativeInterpreterImage saveTo: "phase2.o"'
gcc -o phase2 phase2.o build-support/support-x86_64.o $BUILD_LD_FLAGS
