#!/bin/sh
set -ex

build/dist/sysbvmi module-sources/Bootstrap/main.tlisp module-sources/CLI-Interpreter/main.sysmel -e 'CLISysmelInterpreter buildFullNativeInterpreterImage saveTo: "fullPhase0.o"'
gcc -o fullPhase0 fullPhase0.o build-support/support-x86_64.o
