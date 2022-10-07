# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include_guard(GLOBAL)

option(DUNEPY_COMPILER OFF)


set(DUNEPY_FILE replace_dunepy_replace)

file(
    WRITE ${DUNEPY_FILE}.cc
    "int main() { return 0; }"
)

find_program (CHMOD_PROGRAM chmod)
if(NOT CHMOD_PROGRAM)
  message( SEND_ERROR "chmod not found, please disable ${FLAGSNAMES}")
endif()


file(
  WRITE DUNEPY_compiler.sh
"#!/bin/sh
if ! test -f ${CMAKE_BINARY_DIR}/dp_compiler.sh; then
  compiler=${CMAKE_CXX_COMPILER}
  line=\"$@\"
  # remove CMake specific dirs
  cmakedir=\"CMakeFiles/${DUNEPY_FILE}.dir/\"
  cxxflags=\$\{line/\$\{cmakedir\}/\}
  cxxflags=\$\{cxxflags/\$\{cmakedir\}/\}
  cxxflags=\$\{cxxflags/\$\{cmakedir\}/\}
  cxxflags=\$\{cxxflags/\$\{cmakedir\}/\}
  #/\$\{dunepyobj\}/\}
  echo \"CXXFLAGS=\\\"$cxxflags\\\"\" > ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"MOD=\\\$1\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"MODSTR=\\\"python/dune/generated/\\\$MOD\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"CXXFLAGS=\\\"\\\$\{CXXFLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"CXXFLAGS=\\\"\\\$\{CXXFLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"CXXFLAGS=\\\"\\\$\{CXXFLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"CXXFLAGS=\\\"\\\$\{CXXFLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  #echo \"exec $compiler \\\"\\$CXXFLAGS\\\"\" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  echo \"exec $compiler \\\$CXXFLAGS \" >> ${CMAKE_BINARY_DIR}/dp_compiler.sh
  ${CHMOD_PROGRAM} 755 ${CMAKE_BINARY_DIR}/dp_compiler.sh
elif ! test -f ${CMAKE_BINARY_DIR}/dp_linker.sh; then
  linker=${CMAKE_CXX_COMPILER}
  line=\"$@\"
  cmakedir=\"CMakeFiles/${DUNEPY_FILE}.dir/\"
  flags=\$\{line/\$\{cmakedir\}/\}
  flags=\$\{flags/\$\{cmakedir\}/\}
  flags=\$\{flags/\$\{cmakedir\}/\}
  flags=\$\{flags/\$\{cmakedir\}/\}
  echo \"MOD=\\\$1\" > ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"MODSTR=\\\"\\\$MOD\\\"\" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"FLAGS=\\\"\$\{flags\}\\\"\" >>  ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"FLAGS=\\\"\\\$\{FLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"FLAGS=\\\"\\\$\{FLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"FLAGS=\\\"\\\$\{FLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"FLAGS=\\\"\\\$\{FLAGS/${DUNEPY_FILE}/\\\$\{MODSTR\}\}\\\"\" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  echo \"exec $linker \\\"\\$FLAGS\\\" \" >> ${CMAKE_BINARY_DIR}/dp_linker.sh
  ${CHMOD_PROGRAM} 755 ${CMAKE_BINARY_DIR}/dp_linker.sh
fi
exec ${CMAKE_CXX_COMPILER} \"\$@\""
)
execute_process(COMMAND ${CHMOD_PROGRAM} 755 DUNEPY_compiler.sh)

set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 0)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 0)

# define my own search path for shared libraries
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG}${CMAKE_BINARY_DIR}/lib")

set(CMAKE_CXX_LINK_EXECUTABLE_BAK ${CMAKE_CXX_LINK_EXECUTABLE})
set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/DUNEPY_compiler.sh)

#set(CMAKE_CXX_COMPILER "echo CXXFLAGS: ${DEFAULT_CXXFLAGS} ${CMAKE_CXX_FLAGS} LINK_FLAGS: <LINK_FLAGS> LINK_LIBRARIES: <LINK_LIBRARIES> OBJECTS: <OBJECTS>")

#dune_add_pybind11_module(NAME dunepy)
#set(CMAKE_CXX_LINK_EXECUTABLE_BAK ${CMAKE_CXX_LINK_EXECUTABLE})
#set(CMAKE_CXX_LINK_EXECUTABLE "echo CXXFLAGS: ${DEFAULT_CXXFLAGS} ${CMAKE_CXX_FLAGS} LINK_FLAGS: <LINK_FLAGS> LINK_LIBRARIES: <LINK_LIBRARIES> OBJECTS: <OBJECTS>")

#set_target_properties(dunepy PROPERTIES SKIP_BUILD_RPATH 1)
# dune_target_enable_all_packages(dunepy)

#set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE_BAK})

#set(CMAKE_ECHO_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES})
#set(CMAKE_ECHO_FLAGS ${CMAKE_CXX_FLAGS})
#set(CMAKE_ECHO_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
#set(CMAKE_ECHO_IMPLICIT_LINK_DIRECTORIES ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
#set(
#    CMAKE_ECHO_LINK_EXECUTABLE
#    "<CMAKE_COMMAND> -E echo \"<FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>\" > <TARGET>"
#)

#find_program (ECHO_PROGRAM echo)
#if(NOT ECHO_PROGRAM)
#  message( SEND_ERROR "echo not found, please disable ${FLAGSNAMES}")
#endif()
#mark_as_advanced(ECHO_PROGRAM)


#add_executable(LinkLine dunepy.cc)
#target_link_libraries(LinkLine ${DUNE_LIBS})
#
#set_target_properties(
#    LinkLine
#        PROPERTIES
#            LINKER_LANGUAGE ECHO
#            SUFFIX          ".sh"
#)
