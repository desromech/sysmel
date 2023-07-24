@echo off
md build-support

call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl /I. /Iinclude /c /Z7 /O2 /Gy /MD /W4 /wd4146 /Fobuild-support\support-x86_64.obj lib\support\support.c

call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cl /I. /Iinclude /c /Z7 /O2 /Gy /MD /W4 /wd4146 /Fobuild-support\support-x86.obj lib\support\support.c
