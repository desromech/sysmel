@echo off

build-bootstrap\windows\x64\sysbvmi.exe module-sources/Bootstrap/main.sysmel module-sources/CLI-Interpreter/main.sysmel -e "CLISysmelInterpreter buildNativeInterpreterImage saveTo: ""build-bootstrap/windows/x64/phase0.obj"""
