# -*- mode: shell-script ; coding: utf-8-unix -*-
#! /bin/sh
# export PATH="/cygdrive/c/cygwin-x86_64/tmp/cmake-3.1.0-win32-x86/bin/:$PATH"

rm -rf CMakeCache.txt
rm -rf cmake_install.cmake
rm -rf CMakeFiles
rm -rf Makefile


# cmake --version
# cmake -G "Unix Makefiles" ../clang-server
# cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="c:/cygwin-x86_64/tmp/llvm-build-shells/sh/clang-350/build"
cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="/home/yaruopooner/work/llvm-build-shells/sh/clang-350/build"
# cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="/home/yaruopooner/work/llvm-build-shells/sh/clang-350/build" -DCMAKE_INSTALL_PREFIX="~/work/test"

# echo "please press Enter key"
# read discard_tmp

# cmake --build .
cmake --build . --config Release
# sudo make install


