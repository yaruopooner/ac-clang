@echo off


del /Q CMakeCache.txt
del /Q cmake_install.cmake
rmdir /Q /S CMakeFiles
rmdir /Q /S clang-server-x86_64.dir


set BUILD_OPTIONS=%~f0.ini
echo F | xcopy /D %BUILD_OPTIONS%.sample %BUILD_OPTIONS%


@rem parse for build option
setlocal enabledelayedexpansion


if exist "%BUILD_OPTIONS%" (
   set LINE_PREFIX=
   set INI_SECTION=

   for /f "eol=; delims== tokens=1,2" %%a in ( %BUILD_OPTIONS% ) do (
      set LINE_VALUE=%%a
      set LINE_PREFIX=!LINE_VALUE:~0,1!!LINE_VALUE:~-1,1!
      set INI_SECTION=!LINE_VALUE:~1,-1!

      if not "!LINE_PREFIX!"=="[]" (
         set !LINE_VALUE!=%%b
      )
   )
)



if not "%1" == "" (
   set CLANG_VERSION=%1
)

if not "%2" == "" (
   set VS_VERSION=%2
)

if not "%3" == "" (
   set ARCH=%3
)

if not "%4" == "" (
   set CONFIG=%4
)

if not "%5" == "" (
   set INSTALL_PREFIX=%5
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
  exit /B 1
)


if %ARCH% == 64 (
   set GENERATOR=%GENERATOR% Win64
)


set PATH=%CMAKE_PATH%;%PATH%
set LLVM_LIBRARY_PATHS="%LLVM_BUILD_SHELLS_PATH%/ps1/clang-%CLANG_VERSION%/build/msvc%VS_VERSION%-%ARCH%/%CONFIG%/"


@echo on

@rem goto :end

cmake -G "%GENERATOR%" ../clang-server -DLIBRARY_PATHS=%LLVM_LIBRARY_PATHS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%

@rem @pause

@rem cmake --build . [--config <config>] [--target <target>] [-- -i]
@rem cmake --build . --config %CONFIG% --target ALL_BUILD
cmake --build . --config %CONFIG% --target INSTALL

@rem @pause

@rem :end
@rem set
