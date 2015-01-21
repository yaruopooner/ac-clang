# -*- mode: cmake ; coding: utf-8-unix -*-

cmake_minimum_required( VERSION 2.8)

cmake_policy(SET CMP0015 OLD)
cmake_policy(SET CMP0022 NEW)

include(CheckCXXCompilerFlag)

  
set(platform-name x86_64)
# set(platform-name x86_32)

set(project-name clang-server-${platform-name})

Project(${project-name})

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug Release)

set(project-sources main.cpp ClangServer.cpp ClangServer.hpp ClangSession.cpp ClangSession.hpp Common.cpp Common.hpp)


# link_directories(. ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libclang SHARED IMPORTED)


list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_PREFIXES)

# set(libclang_names libclang-${platform-name})
set(libclang_names clang-${platform-name} clang)


function(display_vars vars prefix)
  message("${prefix}")
  foreach(IT ${vars})
    message("${prefix}${IT} = ${${IT}}")
  endforeach()
endfunction()


set(echo_vars)
list(APPEND echo_vars CMAKE_CXX_COMPILER_ID CMAKE_CXX_STANDARD CYGWIN WIN32 UNIX MSVC MSVC_VERSION CMAKE_GNUtoMS CMAKE_SYSTEM_NAME CMAKE_COMPILER_IS_GNUC CMAKE_COMPILER_IS_GNUCXX CMAKE_COMPILER_IS_MINGW CMAKE_COMPILER_IS_CYGWIN)
display_vars("${echo_vars}" "env: ")


if (MSVC)
  message("environment: MSVC")
  
  set(echo_vars)
  list(APPEND echo_vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
  list(APPEND echo_vars CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE)
  list(APPEND echo_vars IMPORTED_IMPLIB IMPORTED_IMPLIB_DEBUG IMPORTED_IMPLIB_RELEASE)
  display_vars("${echo_vars}" "msvc: ")

  
  # set_target_properties(TARGET)

  # setup compile & link flags
  add_definitions(-D_CONSOLE -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  set(CMAKE_CXX_FLAGS "/GR /EHsc /W3")
  set(CMAKE_CXX_FLAGS_DEBUG "/Gm /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/Zi /GL /Gy /Oi")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /DEBUG /OPT:ICF /OPT:REF")

  
  # search & setup link library properties
  list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .imp)
  list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_SUFFIXES)


  foreach(IT ${LIBRARY_PATHS})
    link_directories(${IT})
  endforeach()
  

  # setup library per configuration
  set(msvc_project_configurations DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)

  # search priority EXTERNAL > CURRENT
  set(libclang_search_paths ${LIBRARY_PATHS} ${CMAKE_CURRENT_SOURCE_DIR})

  foreach(IT_SEARCH_PATH ${libclang_search_paths})
    message("IT_SEARCH_PATH : ${IT_SEARCH_PATH}")

    # file(GLOB_RECURSE collected_lib_path RELATIVE ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.imp ${IT_SEARCH_PATH}/*.lib)
    file(GLOB_RECURSE collected_library_apath ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.imp ${IT_SEARCH_PATH}/*.lib)

    if(collected_library_apath)
      # generate absolute path list
      set(library_apaths ${IT_SEARCH_PATH})
      foreach(IT_PATH ${collected_library_apath})
        get_filename_component(library_dir_path ${IT_PATH} DIRECTORY)
        list(APPEND library_apaths ${library_dir_path})
      endforeach()
      list(REMOVE_DUPLICATES library_apaths)

      # message("collected_library_apath : ${collected_library_apath}")
      message("library_apaths : ${library_apaths}")

      # find library > generate relative path > set property per configuration.
      set(anything_matched)
      foreach(IT_CONFIG ${msvc_project_configurations})
        # collect a absolute path that includes the same configuration name.
        # IT_SEARCH_PATH is always accept.
        set(filtered_library_apaths ${IT_SEARCH_PATH})
        foreach(IT_PATH ${library_apaths})
          string(TOUPPER ${IT_PATH} CHECK_PATH_STR)
          if(${CHECK_PATH_STR} MATCHES /${IT_CONFIG})
            list(APPEND filtered_library_apaths ${IT_PATH})
          endif()
        endforeach()

        find_library(libclang_${IT_CONFIG}_found_apath NAMES ${libclang_names} PATHS ${filtered_library_apaths})

        display_vars("filtered_library_apaths;libclang_${IT_CONFIG}_found_apath" "${IT_CONFIG}: ")

        if(libclang_${IT_CONFIG}_found_apath)
          file(RELATIVE_PATH libclang_${IT_CONFIG}_rpath ${IT_SEARCH_PATH} ${libclang_${IT_CONFIG}_found_apath})
          set(anything_matched 1)
        endif()

        set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_${IT_CONFIG} ${libclang_${IT_CONFIG}_rpath})
      endforeach()

      if(anything_matched)
        message("found library at ${IT_SEARCH_PATH}")
        break()
      endif()
    endif()
  endforeach()

  if(NOT anything_matched)
    message("not found library")
  endif()
  

  set(dependency_libraries odbc32 odbccp32 libclang)

elseif(UNIX)
  message("environment: UNIX")

  # setup compile & link flags
  check_cxx_compiler_flag(-std=c++11 COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag(-std=c++0x COMPILER_SUPPORTS_CXX0X)

  if (COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif (COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(FATAL_ERROR "The compiler has no C++11 support.")
  endif()

  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused-result")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-result")


  # search priority EXTERNAL > CURRENT

  # set_property(TARGET libclang PROPERTY IMPORTED_SONAME libclang-${platform-name}d.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LOCATION ./library/${platform-name}/release/libclang-${platform-name}.a)

  # set(dependency_libraries libclang-${platform-name}.imp)
  # set(dependency_libraries clang-${platform-name}.imp)
  if (CYGWIN)
    message("CYGWIN")
    # set(dependency_libraries clang.dll)
    # set(dependency_libraries libclang.a)

    # find_library(CYG_LIBCLANG_PATH NAMES clang.dll PATHS ${library_apaths})
    # find_library(CYG_LIBCLANG_PATH NAMES libclang.dll clang.dll clang PATHS "/lib" "c:/cygwin-x86_64/lib" )
    # find_library(CYG_LIBCLANG_PATH NAMES libclang.dll PATHS "/lib" "c:/cygwin-x86_64/lib" )
    # find_library(CYG_LIBCLANG_PATH NAMES libclang libclang.dll PATHS "c:/cygwin-x86_64/lib" )
    message("CYG_LIBCLANG_PATH : ${CYG_LIBCLANG_PATH}")

  else()
    message("non CYGWIN")
    # set(dependency_libraries clang)
    # set(dependency_libraries clang-x86_64 clang)
  endif()

  # list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .a)

  foreach(IT ${LIBRARY_PATHS})
    link_directories(${IT})
  endforeach()
  
  # search priority EXTERNAL > CURRENT
  set(libclang_search_paths ${LIBRARY_PATHS} ${CMAKE_CURRENT_SOURCE_DIR})

  foreach(IT_SEARCH_PATH ${libclang_search_paths})
    message("IT_SEARCH_PATH : ${IT_SEARCH_PATH}")

    # file(GLOB_RECURSE collected_library_apath RELATIVE ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.so ${IT_SEARCH_PATH}/*.a)
    file(GLOB_RECURSE collected_library_apath ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.so ${IT_SEARCH_PATH}/*.a)

    if(collected_library_apath)
      # generate relative path list
      set(library_apaths ${IT_SEARCH_PATH})
      foreach(IT_PATH ${collected_library_apath})
        get_filename_component(library_dir_path ${IT_PATH} DIRECTORY)
        list(APPEND library_apaths ${library_dir_path})
      endforeach()
      list(REMOVE_DUPLICATES library_apaths)

      message("collected_library_apath : ${collected_library_apath}")
      message("library_apaths : ${library_apaths}")

      # find library > generate relative path > set property per configuration
      set(anything_matched)
      find_library(libclang_found_path NAMES ${libclang_names} PATHS ${library_apaths})

      if(libclang_found_path)
        file(RELATIVE_PATH libclang_rpath ${IT_SEARCH_PATH} ${libclang_found_path})
        set(anything_matched 1)
      endif()

      # set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_ ${libclang_rpath})
      set(dependency_libraries ${libclang_rpath})

      if(anything_matched)
        message("found library at ${IT_SEARCH_PATH}")
        break()
      endif()
    endif()
  endforeach()

  if(NOT anything_matched)
    message("not found library")
  endif()

  
else()

  message("sorry, not support environment")

endif()


add_executable(${project-name} ${project-sources})

# set_property(TARGET ${project-name} PROPERTY CXX_STANDARD 11)

set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_DEBUG "${project-name}d")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_RELEASE "${project-name}")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_MINSIZEREL "${project-name}")
set_property(TARGET ${project-name} PROPERTY OUTPUT_NAME_RELWITHDEBINFO "${project-name}rd")

# set_property(TARGET ${project-name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ./binary)
# set_property(TARGET ${project-name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ./binary)
# set_property(TARGET ${project-name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ./binary)
# set_property(TARGET ${project-name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ./binary)
# set_property(TARGET ${project-name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ./binary)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./binary)


include_directories(./)


target_link_libraries(${project-name} ${dependency_libraries})


# list(APPEND libclang_library_search_paths ${LLVM_EXTERNAL_PATH} ${CUSTOM_LIBRARY_PATH} ${INSTALLED_LIBRARY_PATH})

# find_library(libclang_found_path NAMES libclang-x86_64 libclang
#   PATHS ${libclang_library_search_paths}
#  )


  
get_target_property(TARGET_PROP_TEST_DEBUG libclang IMPORTED_IMPLIB_DEBUG)
get_target_property(TARGET_PROP_TEST_RELEASE libclang IMPORTED_IMPLIB_RELEASE)


set(echo_vars)
list(APPEND echo_vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
list(APPEND echo_vars CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE)
list(APPEND echo_vars IMPORTED_IMPLIB IMPORTED_IMPLIB_DEBUG IMPORTED_IMPLIB_RELEASE)
list(APPEND echo_vars IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE)
list(APPEND echo_vars FOUND_LIB)
list(APPEND echo_vars CMAKE_FIND_LIBRARY_PREFIXES CMAKE_FIND_LIBRARY_SUFFIXES)
list(APPEND echo_vars CMAKE_IMPORT_LIBRARY_SUFFIXES CMAKE_LINK_LIBRARY_SUFFIX CMAKE_SHARED_LIBRARY_SUFFIX CMAKE_SHARED_MODULE_SUFFIX CMAKE_STATIC_LIBRARY_SUFFIX)
list(APPEND echo_vars dependency_libraries)
list(APPEND echo_vars TARGET_PROP_TEST_DEBUG TARGET_PROP_TEST_RELEASE ARG_TEST CMAKE_CURRENT_SOURCE_DIR collected_library_apath library_apaths)
display_vars("${echo_vars}" "final: ")


# get_filename_component(LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY} DIRECTORY)
# get_filename_component(LIBCLANG_LIBRARY_DIR "./library/${platform-name}/release/libclang-x86_64.imp" DIRECTORY)
# get_filename_component(LIBCLANG_LIBRARY_NAME "./library/${platform-name}/release/libclang-x86_64.imp" NAME)
# get_filename_component(LIBCLANG_LIBRARY_NAME_WE "./library/${platform-name}/release/libclang-x86_64.imp" NAME_WE)
# get_filename_component(LIBCLANG_LIBRARY_APATH "./library/${platform-name}/release/libclang-x86_64.imp" ABSOLUTE)

# message("final:LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY_DIR}")
# message("final:LIBCLANG_LIBRARY_NAME ${LIBCLANG_LIBRARY_NAME}")
# message("final:LIBCLANG_LIBRARY_NAME_WE ${LIBCLANG_LIBRARY_NAME_WE}")
# message("final:LIBCLANG_LIBRARY_APATH ${LIBCLANG_LIBRARY_APATH}")
