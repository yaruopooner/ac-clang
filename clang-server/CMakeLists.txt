# -*- mode: cmake ; coding: utf-8-unix -*-

cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0015 OLD)
cmake_policy(SET CMP0022 NEW)

include(CheckCXXCompilerFlag)

  
set(platform-name x86_64)
# set(platform-name x86_32)
set(project-name clang-server-${platform-name})


Project(${project-name})


set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug Release)

set(sources main.cpp ClangServer.cpp ClangServer.hpp ClangSession.cpp ClangSession.hpp Common.cpp Common.hpp)


link_directories(./library/${platform-name}/)

add_library(libclang SHARED IMPORTED)


message("compiler is : ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_STANDARD is : ${CMAKE_CXX_STANDARD}")
message("CYGWIN : ${CYGWIN}")
message("WIN32 : ${WIN32}")
message("UNIX : ${UNIX}")
message("MSVC : ${MSVC}")
message("MSVC_VERSION : ${MSVC_VERSION}")
message("CMAKE_GNUtoMS : ${CMAKE_GNUtoMS}")
message("CMAKE_SYSTEM_NAME : ${CMAKE_SYSTEM_NAME}")
message("CMAKE_COMPILER_IS_GNUC   : ${CMAKE_COMPILER_IS_GNUC}")
message("CMAKE_COMPILER_IS_GNUCXX : ${CMAKE_COMPILER_IS_GNUCXX}")
message("CMAKE_COMPILER_IS_MINGW  : ${CMAKE_COMPILER_IS_MINGW}") 
message("CMAKE_COMPILER_IS_CYGWIN : ${CMAKE_COMPILER_IS_CYGWIN}")

if ( MSVC )

  message( "msvc:" )
  message( "msvc:CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}" )
  message( "msvc:CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}" )
  message( "msvc:CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}" )
  message( "msvc:CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}" )
  message( "msvc:CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG}" )
  message( "msvc:CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELEASE}" )
  message( "msvc:IMPORTED_IMPLIB ${IMPORTED_IMPLIB}" )
  message( "msvc:IMPORTED_IMPLIB_DEBUG ${IMPORTED_IMPLIB_DEBUG}" )
  message( "msvc:IMPORTED_IMPLIB_RELEASE ${IMPORTED_IMPLIB_RELEASE}" )

  # set_target_properties(TARGET)

  add_definitions(-D_CONSOLE -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  set(CMAKE_CXX_FLAGS "/GR /EHsc /W3")
  set(CMAKE_CXX_FLAGS_DEBUG "/Gm /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/Zi /GL /Gy /Oi")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /DEBUG /OPT:ICF /OPT:REF")

  set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_DEBUG libclang-${platform-name}d.imp)
  set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_RELEASE libclang-${platform-name}.imp)
  set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_MINSIZEREL libclang-${platform-name}.imp)
  set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO libclang-${platform-name}rd.imp)

  set(dependency_libraries odbc32 odbccp32 libclang)

elseif( UNIX )

  message( "UNIX" )

  check_cxx_compiler_flag(-std=c++11 COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag(-std=c++0x COMPILER_SUPPORTS_CXX0X)

  if ( COMPILER_SUPPORTS_CXX11 )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif ( COMPILER_SUPPORTS_CXX0X )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message( FATAL_ERROR "The compiler has no C++11 support." )
  endif()

  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused-result")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-result")

  link_directories(./library/${platform-name}/release)

  # set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_DEBUG libclang-${platform-name}d.imp)
  # set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_RELEASE libclang-${platform-name}.imp)
  # set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_MINSIZEREL libclang-${platform-name}.imp)
  # set_property(TARGET libclang APPEND PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO libclang-${platform-name}rd.imp)

  # set_property(TARGET libclang PROPERTY IMPORTED_SONAME libclang-${platform-name}.so)
  # set_property(TARGET libclang PROPERTY IMPORTED_SONAME_DEBUG libclang-${platform-name}d.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LOCATION libclang-${platform-name}.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES libclang-${platform-name}d.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LOCATION_DEBUG ./library/${platform-name}/debug/libclang-${platform-name}d.a)

  # set_property(TARGET libclang PROPERTY IMPORTED_SONAME libclang-${platform-name}d.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LOCATION ./library/${platform-name}/release/libclang-${platform-name}.a)

  # set(dependency_libraries libclang-${platform-name}.imp)
  # set(dependency_libraries clang-${platform-name}.imp)
  if ( CYGWIN )
    message( "CYGWIN" )
    set(dependency_libraries clang.dll)
  else()
    message( "non CYGWIN" )
    set(dependency_libraries clang)
    # set(dependency_libraries clang-x86_64 clang)
  endif()

else()

  message( "other" )

endif()


add_executable(${project-name} ${sources})

# set_property(TARGET ${project-name} PROPERTY CXX_STANDARD 11)

set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_DEBUG "${project-name}d")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_RELEASE "${project-name}")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_MINSIZEREL "${project-name}")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_RELWITHDEBINFO "${project-name}rd")


include_directories(./)


target_link_libraries(${project-name} ${dependency_libraries})


message( "final:" )
message( "final:CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}" )
message( "final:CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}" )
message( "final:CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}" )
message( "final:CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}" )
message( "final:CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG}" )
message( "final:CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELEASE}" )
message( "final:IMPORTED_IMPLIB ${IMPORTED_IMPLIB}" )
message( "final:IMPORTED_IMPLIB_DEBUG ${IMPORTED_IMPLIB_DEBUG}" )
message( "final:IMPORTED_IMPLIB_RELEASE ${IMPORTED_IMPLIB_RELEASE}" )
message( "final:IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG ${IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG}" )
message( "final:IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE ${IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE}" )
message( "final:LINK_INTERFACE_LIBRARIES_DEBUG ${LINK_INTERFACE_LIBRARIES_DEBUG}" )
message( "final:LINK_INTERFACE_LIBRARIES_RELEASE ${LINK_INTERFACE_LIBRARIES_RELEASE}" )




