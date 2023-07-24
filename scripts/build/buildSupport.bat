@echo off
md build-support

call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl /I. /Iinclude /c /Z7 /O2 /Gy /MD /W4 /wd4146 /Fobuild-bootstrap\windows\x64\sysmel-pal.obj lib\support\support.c

call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cl /I. /Iinclude /c /Z7 /O2 /Gy /MD /W4 /wd4146 /Fobuild-bootstrap\windows\x86\sysmel-pal.obj lib\support\support.c
