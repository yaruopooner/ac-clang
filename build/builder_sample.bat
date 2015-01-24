@rem set PATH=c:/cygwin-x86_64/tmp/cmake-3.0.2-win32-x86/bin;%PATH%
set PATH=c:/cygwin-x86_64/tmp/cmake-3.1.0-win32-x86/bin;%PATH%


del /Q CMakeFiles
del /Q Makefiles
del /Q CMakeCache.txt
del /Q cmake_install.cmake


@rem cmake clean .
@rem cmake -G "Visual Studio 12 2013 Win64" ../clang-server -DLIBRARY_PATHS="c:/cygwin-x86_64/tmp/llvm-build-shells/ps1/clang-350/build/msvc-64/"
cmake -G "Visual Studio 12 2013 Win64" ../clang-server

pause


@rem cmake --build . --target INSTALL -- /verbosity:detailed
@rem  <cmake> --build . [--config <config>] [--target <target>] [-- -i]
@rem cmake --build . --config Release
@rem cmake --build . --config Release --target clang-server-x86_64
cmake --build . --config Release --target ALL_BUILD
@rem cmake --build . --config Debug --target ALL_BUILD

pause
