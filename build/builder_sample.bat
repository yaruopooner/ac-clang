@echo off
rem set PATH=c:/cygwin-x86_64/tmp/cmake-3.0.2-win32-x86/bin;%PATH%
set PATH=c:/cygwin-x86_64/tmp/llvm-build-shells/ps1/tools-latest-version/cmake-3.6.2-win64-x64/bin;%PATH%

del /Q CMakeCache.txt
del /Q cmake_install.cmake
rmdir /Q /S CMakeFiles
rmdir /Q /S clang-server-x86_64.dir

if "%1" == "" (
   set CLANG_VERSION=390
) else (
   set CLANG_VERSION=%1
)

if "%2" == "" (
   set VS_VERSION=2015
) else (
   set VS_VERSION=%2
)

if "%3" == "" (
   set ARCH=64
) else (
   set ARCH=%3
)

if "%4" == "" (
   set CONFIG=Release
) else (
   set CONFIG=%4
)

if "%5" == "" (
   set INSTALL="c:/cygwin-x86_64/usr/local/bin/"
) else (
   set INSTALL=%5
)


set GENERATOR=Visual Studio
if %VS_VERSION% == 2017 (
   set GENERATOR=%GENERATOR% 15 2017
) else if %VS_VERSION% == 2015 (
   set GENERATOR=%GENERATOR% 14 2015
) else if %VS_VERSION% == 2013 (
   set GENERATOR=%GENERATOR% 12 2013
) else if %VS_VERSION% == 2012 (
   set GENERATOR=%GENERATOR% 11 2012
) else (
  echo unsupported Visual Studio Version!
  exit
)

if %ARCH% == 64 (
   set GENERATOR=%GENERATOR% Win64
)



set LLVM_LIBRARY_PATHS="c:/cygwin-x86_64/tmp/llvm-build-shells/ps1/clang-%CLANG_VERSION%/build/msvc%VS_VERSION%-%ARCH%/%CONFIG%/"
@rem set INSTALL_PREFIX="c:/cygwin-x86_64/usr/local/bin/"
set INSTALL_PREFIX=%INSTALL%

@echo on


cmake -G "%GENERATOR%" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%

@rem @pause

@rem cmake --build . [--config <config>] [--target <target>] [-- -i]
@rem cmake --build . --config %CONFIG% --target ALL_BUILD
cmake --build . --config %CONFIG% --target INSTALL

@rem @pause
