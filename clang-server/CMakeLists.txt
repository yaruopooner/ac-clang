# -*- mode: cmake ; coding: utf-8-unix -*-

cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0015 OLD)
cmake_policy(SET CMP0022 NEW)

include(CheckCXXCompilerFlag)


# ----------------------------------------
# before common setting
# ----------------------------------------

set(project_name clang-server)
set(platform_name x86_64)
# set(platform_name x86_32)
set(project_elf_name ${project_name}-${platform_name})


# CMAKE_INSTALL_PREFIX check designated value before function Project()
if(CMAKE_INSTALL_PREFIX)
  # message("user_install_prefix TRUE")
  set(user_install_prefix TRUE)
else()
  # message("user_install_prefix FALSE")
endif()


Project(${project_name})

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug Release)


# setup library per configuration
set(project_configurations DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)


# CMAKE_INSTALL_PREFIX 
# Windows (exe,dll)
# Default: C:/Program Files/clang-server/
# prefix : <user designate directory>
# 
# Linux (elf)
# Default: /usr/local/bin/
# prefix : <user designate directory>
# Linux (so)
# Default: /usr/local/lib/clang-server/
# prefix : <user designate directory>

set(CMAKE_SKIP_RPATH TRUE)
# set(CMAKE_SKIP_BUILD_RPATH FALSE)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)


set(project-sources main.cpp ClangServer.cpp ClangServer.hpp ClangSession.cpp ClangSession.hpp Common.cpp Common.hpp)


add_library(libclang SHARED IMPORTED)

set(dependency_libraries libclang)


list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_PREFIXES)

# set(libclang_names libclang-${platform_name})
set(libclang_names clang-${platform_name} clang)


function(display_vars vars prefix)
  message("${prefix}")
  foreach(IT ${vars})
    message("${prefix}${IT} = ${${IT}}")
  endforeach()
endfunction()


set(echo_vars)
list(APPEND echo_vars CMAKE_CXX_COMPILER_ID CMAKE_CXX_STANDARD CYGWIN WIN32 UNIX MSVC MSVC_VERSION CMAKE_GNUtoMS CMAKE_SYSTEM_NAME CMAKE_COMPILER_IS_GNUC CMAKE_COMPILER_IS_GNUCXX CMAKE_COMPILER_IS_MINGW CMAKE_COMPILER_IS_CYGWIN)
display_vars("${echo_vars}" "env: ")


# ----------------------------------------
# platform dependency setting
# ----------------------------------------

if(MSVC)
  # ----------------------------------------
  # Microsoft Visual Studio
  # ----------------------------------------
  message("environment: MSVC")
  
  set(echo_vars)
  list(APPEND echo_vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
  list(APPEND echo_vars CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE)
  list(APPEND echo_vars IMPORTED_IMPLIB IMPORTED_IMPLIB_DEBUG IMPORTED_IMPLIB_RELEASE)
  display_vars("${echo_vars}" "msvc: ")

  
  # set_target_properties(TARGET)

  # setup compile & link flags
  add_definitions(-D_CONSOLE -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  set(CMAKE_CXX_FLAGS "/GR /EHsc /W3 /Zi")
  set(CMAKE_CXX_FLAGS_DEBUG "/Gm /Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/GL /Gy /Oi")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /DEBUG /OPT:ICF /OPT:REF")

  set(dependency_libraries odbc32 odbccp32 ${dependency_libraries})

  
  # search & setup link library properties
  list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .imp)
  list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_SUFFIXES)


  # search priority EXTERNAL > CURRENT
  set(libclang_search_paths ${LIBRARY_PATHS} ${CMAKE_SOURCE_DIR})

  foreach(IT_SEARCH_PATH ${libclang_search_paths})
    message("IT_SEARCH_PATH : ${IT_SEARCH_PATH}")

    # collect absolute file paths
    file(GLOB_RECURSE collected_library_apath ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.imp ${IT_SEARCH_PATH}/*.lib ${IT_SEARCH_PATH}/*.dll)

    if(collected_library_apath)
      # generate absolute directory paths
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
      foreach(IT_CONFIG ${project_configurations})
        # collect a absolute path that includes the same configuration name.
        # IT_SEARCH_PATH is always accept.
        set(filtered_library_apaths ${IT_SEARCH_PATH})
        foreach(IT_PATH ${library_apaths})
          string(TOUPPER ${IT_PATH} CHECK_PATH_STR)
          if(${CHECK_PATH_STR} MATCHES /${IT_CONFIG})
            list(APPEND filtered_library_apaths ${IT_PATH})
          endif()
        endforeach()

        set(CMAKE_FIND_LIBRARY_SUFFIXES .imp .lib)
        find_library(libclang_${IT_CONFIG}_found_lib_apath NAMES ${libclang_names} PATHS ${filtered_library_apaths} NO_DEFAULT_PATH)

        set(CMAKE_FIND_LIBRARY_SUFFIXES .dll)
        find_library(libclang_${IT_CONFIG}_found_dll_apath NAMES ${libclang_names} PATHS ${filtered_library_apaths} NO_DEFAULT_PATH)

        display_vars("filtered_library_apaths;libclang_${IT_CONFIG}_found_lib_apath;libclang_${IT_CONFIG}_found_dll_apath" "${IT_CONFIG}: ")

        if(libclang_${IT_CONFIG}_found_lib_apath AND libclang_${IT_CONFIG}_found_dll_apath)
          file(RELATIVE_PATH libclang_${IT_CONFIG}_rpath ${IT_SEARCH_PATH} ${libclang_${IT_CONFIG}_found_lib_apath})
          set(anything_matched 1)
        endif()

        set_property(TARGET libclang PROPERTY IMPORTED_IMPLIB_${IT_CONFIG} ${libclang_${IT_CONFIG}_rpath})
        set_property(TARGET libclang PROPERTY INTERFACE_LINK_LIBRARIES_${IT_CONFIG} ${libclang_${IT_CONFIG}_rpath})
        set_property(TARGET libclang PROPERTY IMPORTED_LOCATION_${IT_CONFIG} ${libclang_${IT_CONFIG}_found_lib_apath})
      endforeach()

      if(anything_matched)
        message("found library at ${IT_SEARCH_PATH}")
        link_directories(${IT_SEARCH_PATH})
        break()
      endif()
    endif()
  endforeach()


  # install info
  set(binary_destination_apath ${CMAKE_INSTALL_PREFIX})
  set(library_destination_apath ${CMAKE_INSTALL_PREFIX})

  
  if(NOT anything_matched)
    message("not found library")
  endif()

elseif(UNIX)
  # ----------------------------------------
  # UNIX, Linux
  # ----------------------------------------
  message("environment: UNIX")

  # setup compile & link flags
  check_cxx_compiler_flag(-std=c++11 COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag(-std=c++0x COMPILER_SUPPORTS_CXX0X)

  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif (COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(FATAL_ERROR "The compiler has no C++11 support.")
  endif()

  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused-result")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-result")


  # search priority EXTERNAL > CURRENT

  if(CYGWIN)
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
  endif()


  # search priority EXTERNAL > CURRENT
  set(libclang_search_paths ${LIBRARY_PATHS} ${CMAKE_SOURCE_DIR})

  foreach(IT_SEARCH_PATH ${libclang_search_paths})
    message("IT_SEARCH_PATH : ${IT_SEARCH_PATH}")

    # collect absolute file paths
    file(GLOB_RECURSE collected_library_apath ${IT_SEARCH_PATH} ${IT_SEARCH_PATH}/*.so ${IT_SEARCH_PATH}/*.a)

    if(collected_library_apath)
      # generate absolute directory paths
      set(library_apaths ${IT_SEARCH_PATH})
      foreach(IT_PATH ${collected_library_apath})
        get_filename_component(library_dir_path ${IT_PATH} DIRECTORY)
        list(APPEND library_apaths ${library_dir_path})
      endforeach()
      list(REMOVE_DUPLICATES library_apaths)

      # message("collected_library_apath : ${collected_library_apath}")
      message("library_apaths : ${library_apaths}")

      # find library > generate relative path
      find_library(libclang_found_apath NAMES ${libclang_names} PATHS ${library_apaths} NO_DEFAULT_PATH)

      display_vars("libclang_found_apath" " : ")
      if(libclang_found_apath)
        message("found library at ${libclang_found_apath}")
        get_filename_component(libclang_link_apath ${libclang_found_apath} DIRECTORY)
        link_directories(${libclang_link_apath})
        get_filename_component(libclang_found_name ${libclang_found_apath} NAME_WE)
        string(REGEX REPLACE "^lib" "" libclang_found_name ${libclang_found_name})

        # message("libclang_found_name at ${libclang_found_name}")
        set_property(TARGET libclang PROPERTY INTERFACE_LINK_LIBRARIES ${libclang_found_name})
        set_property(TARGET libclang PROPERTY IMPORTED_LOCATION ${libclang_found_apath})
        break()
      endif()
    endif()
  endforeach()


  # install info
  if(user_install_prefix)
    string(REGEX REPLACE "^~" "$ENV{HOME}" CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    get_filename_component(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE)
    set(binary_destination_apath ${CMAKE_INSTALL_PREFIX})
    set(library_destination_apath ${CMAKE_INSTALL_PREFIX})
  else()
    set(binary_destination_apath ${CMAKE_INSTALL_PREFIX}/bin)
    set(library_destination_apath ${CMAKE_INSTALL_PREFIX}/lib/${project_name})
  endif()

  
  # Absolute path of a shared object that executable file is referenced at the runtime.
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,${library_destination_apath}")

  
  if(NOT libclang_found_apath)
    message("not found library")
  endif()

  
else()

  message("sorry, not support environment")

endif()



# ----------------------------------------
# after common setting
# ----------------------------------------

add_executable(${project_name} ${project-sources})

# set_property(TARGET ${project_name} PROPERTY CXX_STANDARD 11)

set_property(TARGET ${project_name} PROPERTY OUTPUT_NAME_DEBUG "${project_elf_name}d")
set_property(TARGET ${project_name} PROPERTY OUTPUT_NAME_RELEASE "${project_elf_name}")
set_property(TARGET ${project_name} PROPERTY OUTPUT_NAME_MINSIZEREL "${project_elf_name}")
set_property(TARGET ${project_name} PROPERTY OUTPUT_NAME_RELWITHDEBINFO "${project_elf_name}rd")


include_directories(./)


target_link_libraries(${project_name} ${dependency_libraries})


# ----------------------------------------
# install setting
# ----------------------------------------

set(CMAKE_INSTALL_RPATH ${library_destination_apath})
install(TARGETS ${project_name} RUNTIME DESTINATION ${binary_destination_apath})
install(FILES ${libclang_found_apath} DESTINATION ${library_destination_apath})
foreach(IT_CONFIG ${project_configurations})
  install(FILES ${libclang_${IT_CONFIG}_found_dll_apath} DESTINATION ${library_destination_apath} CONFIGURATIONS ${IT_CONFIG})
endforeach()



# ----------------------------------------
# debug display for variables
# ----------------------------------------

set(echo_vars)
list(APPEND echo_vars CMAKE_CURRENT_SOURCE_DIR collected_library_apath library_apaths)
list(APPEND echo_vars CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
list(APPEND echo_vars CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE)
list(APPEND echo_vars dependency_libraries binary_destination_apath library_destination_apath)
list(APPEND echo_vars IMPORTED_IMPLIB IMPORTED_IMPLIB_DEBUG IMPORTED_IMPLIB_RELEASE)
list(APPEND echo_vars INTERFACE_LINK_LIBRARIES INTERFACE_LINK_LIBRARIES_DEBUG INTERFACE_LINK_LIBRARIES_RELEASE)
list(APPEND echo_vars IMPORTED_LOCATION IMPORTED_LOCATION_DEBUG IMPORTED_LOCATION_RELEASE)
# list(APPEND echo_vars CMAKE_FIND_LIBRARY_PREFIXES CMAKE_FIND_LIBRARY_SUFFIXES)
# list(APPEND echo_vars CMAKE_IMPORT_LIBRARY_SUFFIXES CMAKE_LINK_LIBRARY_SUFFIX CMAKE_SHARED_LIBRARY_SUFFIX CMAKE_SHARED_MODULE_SUFFIX CMAKE_STATIC_LIBRARY_SUFFIX)
# list(APPEND echo_vars PROJECT_SOURCE_DIR PROJECT_BINARY_DIR CMAKE_SOURCE_DIR CMAKE_BINARY_DIR CMAKE_RUNTIME_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_INSTALL_NAME_DIR CMAKE_INCLUDE_CURRENT_DIR CMAKE_HOME_DIRECTORY CMAKE_CURRENT_LIST_DIR CMAKE_CURRENT_BINARY_DIR CMAKE_BINARY_DIR CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${project_name}_SOURCE_DIR ${project_name}_BINARY_DIR)
list(APPEND echo_vars CMAKE_INSTALL_PREFIX CMAKE_INSTALL_RPATH CMAKE_SKIP_BUILD_RPATH CMAKE_BUILD_WITH_INSTALL_RPATH CMAKE_SKIP_RPATH CMAKE_SKIP_INSTALL_RPATH)
# list(APPEND echo_vars CMAKE_PREFIX_PATH CMAKE_LIBRARY_PATH CMAKE_FRAMEWORK_PATH CMAKE_SYSTEM_PREFIX_PATH CMAKE_SYSTEM_LIBRARY_PATH CMAKE_SYSTEM_FRAMEWORK_PATH)
display_vars("${echo_vars}" "final: ")


