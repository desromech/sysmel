@echo off

build-bootstrap\windows\x64\sysbvmi.exe package-sources/BootstrapPackages.sysmel package-sources/Program.CLI-Interpreter/package.sysmel -e "CLISysmelInterpreter buildNativeInterpreterImage saveTo: ""build-bootstrap/windows/x64/phase0.obj"""
