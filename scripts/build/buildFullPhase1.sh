#!/bin/sh
set -ex

. ./buildCommon.sh

./fullPhase0 -e 'CLISysmelInterpreter buildFullNativeInterpreterImage saveTo: "fullPhase1.o"'
gcc -o fullPhase1 fullPhase1.o build-support/support-x86_64.o $BUILD_LD_FLAGS
