# -*- mode: shell-script ; coding: utf-8-unix -*-
#! /bin/sh
# export PATH="/cygdrive/c/cygwin-x86_64/tmp/cmake-3.1.0-win32-x86/bin/:$PATH"

rm -rf CMakeCache.txt
rm -rf cmake_install.cmake
rm -rf CMakeFiles
rm -rf Makefile

# switch compiler
# export CC=clang
# export CXX=clang++
declare -r CLANG_VERSION="${1-390}"
declare -r LLVM_LIBRARY_PATHS="${HOME}/work/llvm-build-shells/sh/clang-${CLANG_VERSION}/build-Release"
declare -r INSTALL_PREFIX="${HOME}/work/test"

declare -p CLANG_VERSION
declare -p LLVM_LIBRARY_PATHS
declare -p INSTALL_PREFIX


# cmake --version
# cmake -G "Unix Makefiles" ../clang-server -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="${LLVM_LIBRARY_PATHS}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="${LLVM_LIBRARY_PATHS}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=RELWITHDEBINFO
# cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="${LLVM_LIBRARY_PATHS}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=DEBUG
# cmake -G "Unix Makefiles" ../clang-server -DLIBRARY_PATHS="${LLVM_LIBRARY_PATHS}" -DCMAKE_INSTALL_PREFIX="~/work/test" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# echo "please press Enter key"
# read discard_tmp

# cmake --build .
cmake --build . --config Release
# sudo make install


