@rem @set PATH=c:/cygwin-x86_64/tmp/cmake-3.0.2-win32-x86/bin;%PATH%
@set PATH=c:/cygwin-x86_64/tmp/cmake-3.6.2-win64-x64/bin;%PATH%


@del /Q CMakeCache.txt
@del /Q cmake_install.cmake
@rmdir /Q /S CMakeFiles
@rmdir /Q /S clang-server-x86_64.dir

if "%1" == "" (
   @set CLANG_VERSION=390
) else (
   @set CLANG_VERSION=%1
)
@set LLVM_LIBRARY_PATHS="c:/cygwin-x86_64/tmp/llvm-build-shells/ps1/clang-%CLANG_VERSION%/build/msvc2015-64/Release/"
@set INSTALL_PREFIX="c:/cygwin-x86_64/usr/local/bin/"


@rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server
@rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
@rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS%
rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
cmake -G "Visual Studio 14 2015 Win64" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
rem cmake -G "Visual Studio 12 2013" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
@pause

@rem cmake --build . [--config <config>] [--target <target>] [-- -i]
@rem cmake --build . --config Release --target ALL_BUILD
@rem cmake --build . --config Debug --target ALL_BUILD
cmake --build . --config Release --target INSTALL
@rem cmake --build . --config Debug --target INSTALL

@pause
