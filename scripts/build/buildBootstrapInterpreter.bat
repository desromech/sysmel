@echo off
md build-bootstrap\windows\x64
REM call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
"%ProgramFiles%\LLVM\bin\clang-cl.exe" --target=x86_64-pc-windows-msvc /Z7 /O2 /Gy /MD /I. /Iinclude  /DBUILD_SYSBVM_STATIC /W4 /wd4146 /wd4702 /wd4152 /wd4201 /Febuild-bootstrap\windows\x64\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c
REM cl /std:c11 /Z7 /O2 /Gy /MD /I. /Iinclude  /DBUILD_SYSBVM_STATIC /W4 /wd4146 /wd4702 /wd4152 /wd4201 /Febuild-bootstrap\windows\x64\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c

md build-bootstrap\windows\x86
REM call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
"%ProgramFiles%\LLVM\bin\clang-cl.exe" --target=i686-pc-windows-msvc /Z7 /O2 /Gy /MD /I. /Iinclude  /DBUILD_SYSBVM_STATIC /W4 /wd4146 /wd4702 /wd4152 /wd4201 /Febuild-bootstrap\windows\x86\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c
REM cl /std:c11 /Z7 /O2 /Gy /MD /I. /Iinclude  /DBUILD_SYSBVM_STATIC /W4 /wd4146 /wd4702 /wd4152 /wd4201 /Febuild-bootstrap\windows\x86\sysbvmi apps\interpreter\interpreter.c lib\sysbvm\unityBuild.c
