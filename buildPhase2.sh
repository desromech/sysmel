#!/bin/sh
set -ex

./fullPhase1 -e 'CLISysmelInterpreter buildNativeInterpreterImage saveTo: "phase2.o"'
gcc -o phase2 phase1.o build-support/support-x86_64.o
