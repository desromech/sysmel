#!/bin/sh
set -ex

build/dist/sysbvmi -nogc module-sources/Bootstrap/main.sysmel module-sources/CLI-Interpreter/main.sysmel -e 'CLISysmelInterpreter buildNativeInterpreterImage saveTo: "phase0.o"'
gcc -o phase0 phase0.o build-support/support-x86_64.o
