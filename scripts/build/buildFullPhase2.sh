#!/bin/sh
set -ex

. ./buildCommon.sh

./fullPhase1 -e 'CLISysmelInterpreter buildFullNativeInterpreterImage saveTo: "fullPhase2.o"'
gcc -o fullPhase2 fullPhase2.o build-support/support-x86_64.o $BUILD_LD_FLAGS
