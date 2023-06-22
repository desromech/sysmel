#!/bin/sh
set -ex

./fullPhase1 -nogc module-sources/Bootstrap/main.tlisp module-sources/CLI-Interpreter/main.sysmel -e 'CLISysmelInterpreter buildFullNativeInterpreterImage saveTo: "fullPhase2.o"'
gcc -o fullPhase2 fullPhase1.o build-support/support-x86_64.o
