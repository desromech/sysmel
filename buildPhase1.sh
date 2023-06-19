#!/bin/sh
set -ex

./fullPhase0 -e 'CLISysmelInterpreter buildNativeInterpreterImage saveTo: "phase1.o"'
gcc -o phase1 phase1.o build-support/support-x86_64.o
