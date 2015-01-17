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
  set(custom_libclang_name libclang-${platform-name})

  list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .imp)

  file(GLOB_RECURSE collect_lib_path RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/library/*.imp ${CMAKE_CURRENT_SOURCE_DIR}/library/*.lib)

  if(collect_lib_path)
    # generate relative path list
    foreach(IT ${collect_lib_path})
      get_filename_component(library_dir_rpath ${IT} DIRECTORY)
      list(APPEND library_dir_rpath_list ${library_dir_rpath})
    endforeach()

    list(REMOVE_DUPLICATES library_dir_rpath_list)

    # setup library name per configuration
    set(custom_libclang_DEBUG_name ${custom_libclang_name}d)
    set(custom_libclang_RELEASE_name ${custom_libclang_name})
    set(custom_libclang_MINSIZEREL_name ${custom_libclang_name})
    set(custom_libclang_RELWITHDEBINFO_name ${custom_libclang_name}rd)

    set(msvc_project_configurations DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)

    # find library > generate relative path > set property per configuration
    foreach(IT ${msvc_project_configurations})
      find_library(libclang_${IT}_found_path NAMES ${custom_libclang_${IT}_name} PATHS ${library_dir_rpath_list})
      if(libclang_${IT}_found_path)
        file(RELATIVE_PATH libclang_${IT}_rpath ${CMAKE_CURRENT_SOURCE_DIR} ${libclang_${IT}_found_path})
      endif()
      set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_${IT} ${libclang_${IT}_rpath})
    endforeach()
  endif()
  

  set(dependency_libraries odbc32 odbccp32 libclang)

elseif(UNIX)
  message("environment: UNIX")

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

  link_directories(./library/${platform-name}/release)


  # set_property(TARGET libclang PROPERTY IMPORTED_SONAME libclang-${platform-name}d.a)
  # set_property(TARGET libclang PROPERTY IMPORTED_LOCATION ./library/${platform-name}/release/libclang-${platform-name}.a)

  # set(dependency_libraries libclang-${platform-name}.imp)
  # set(dependency_libraries clang-${platform-name}.imp)
  if (CYGWIN)
    message("CYGWIN")
    set(dependency_libraries clang.dll)
  else()
    message("non CYGWIN")
    set(dependency_libraries clang)
    # set(dependency_libraries clang-x86_64 clang)
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
list(APPEND echo_vars TARGET_PROP_TEST_DEBUG TARGET_PROP_TEST_RELEASE ARG_TEST CMAKE_CURRENT_SOURCE_DIR collect_lib_path library_dir_rpath_list)
display_vars("${echo_vars}" "final: ")


# get_filename_component(LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY} DIRECTORY)
get_filename_component(LIBCLANG_LIBRARY_DIR "./library/${platform-name}/release/libclang-x86_64.imp" DIRECTORY)
get_filename_component(LIBCLANG_LIBRARY_NAME "./library/${platform-name}/release/libclang-x86_64.imp" NAME)
get_filename_component(LIBCLANG_LIBRARY_NAME_WE "./library/${platform-name}/release/libclang-x86_64.imp" NAME_WE)
get_filename_component(LIBCLANG_LIBRARY_APATH "./library/${platform-name}/release/libclang-x86_64.imp" ABSOLUTE)

message("final:LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY_DIR}")
message("final:LIBCLANG_LIBRARY_NAME ${LIBCLANG_LIBRARY_NAME}")
message("final:LIBCLANG_LIBRARY_NAME_WE ${LIBCLANG_LIBRARY_NAME_WE}")
message("final:LIBCLANG_LIBRARY_APATH ${LIBCLANG_LIBRARY_APATH}")
