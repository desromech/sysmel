@echo off
md build-fast

md build-fast\x64
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl /I. /Iinclude /Z7 /O2 /Gy /MD /W4 /DBUILD_SYSBVM_STATIC /wd4221 /Febuild-fast\x64\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c

md build-fast\x86
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cl /I. /Iinclude /Z7 /O2 /Gy /MD /W4 /DBUILD_SYSBVM_STATIC /wd4221 /Febuild-fast\x86\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c

