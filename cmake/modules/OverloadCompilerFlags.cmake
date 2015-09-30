# check whether the user wants to overload compile flags upon calling make
#
# Provides the following macros:
#
#   initialize_compiler_script() : needs to be called before further flags are added to CMAKE_CXX_FLAGS
#   finalize_compiler_script()   : needs to be called at the end of the cmake macros, e.g. in finalize_dune_project
#
# Those two macro calls are hooked into dune_project/finalize_dune_project.
#
# .. cmake_variable:: ALLOW_CXXFLAGS_OVERWRITE
#
#    Setting this option will allow you to overload preprocessor definitions from
#    the command line, as it was possible naturally with the autotools build system.
#    This feature only works with a :code:`Unix Makefiles` based generator. You can
#    use it as:
#
#    :code:`make CXXFLAGS="your flags" GRIDTYPE="grid type"`
#
#    :code:`GRIDTYPE` can be anything defined in :code:`config.h` via the :ref:`dune_define_gridtype` macro from dune-grid.
#    Furthermore any CPP variable of the form :code:`-DVAR=VALUE` can be overloaded on the command line.
#
#    .. note::
#       If you don't know what this is or what it's good for, don't use it.
#

option(ALLOW_CXXFLAGS_OVERWRITE OFF)

macro(find_extended_unix_commands)
  include(FindUnixCommands)
  find_program (GREP_PROGRAM grep)
  if(NOT GREP_PROGRAM)
    message( SEND_ERROR "grep not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  find_program (SED_PROGRAM  sed)
  if(NOT SED_PROGRAM)
    message( SEND_ERROR "sed not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  find_program (CUT_PROGRAM  cut)
  if(NOT CUT_PROGRAM)
    message( SEND_ERROR "cut not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  find_program (ENV_PROGRAM  env)
  if(NOT ENV_PROGRAM)
    message( SEND_ERROR "env not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  find_program (ECHO_PROGRAM echo)
  if(NOT ECHO_PROGRAM)
    message( SEND_ERROR "echo not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  find_program (CHMOD_PROGRAM chmod)
  if(NOT CHMOD_PROGRAM)
    message( SEND_ERROR "chmod not found, please disable ALLOW_CXXFLAGS_OVERWRITE")
  endif()
  mark_as_advanced(GREP_PROGRAM)
  mark_as_advanced(SED_PROGRAM)
  mark_as_advanced(CUT_PROGRAM)
  mark_as_advanced(ENV_PROGRAM)
  mark_as_advanced(ECHO_PROGRAM)
  mark_as_advanced(CHMOD_PROGRAM)
endmacro(find_extended_unix_commands)

# init compiler script and store CXX flags
macro(initialize_compiler_script)
  if(ALLOW_CXXFLAGS_OVERWRITE AND (${CMAKE_GENERATOR} MATCHES ".*Unix Makefiles.*"))
    # check for unix commands necessary
    find_extended_unix_commands()
    # set CXXFLAGS as environment variable
    set( DEFAULT_CXXFLAGS ${CMAKE_CXX_FLAGS})
    set( CMAKE_CXX_FLAGS "" )
    set( DEFAULT_CXX_COMPILER ${CMAKE_CXX_COMPILER} )
    set( COMPILER_SCRIPT_FILE "#!${BASH}\nexec ${CMAKE_CXX_COMPILER} \"\$@\"")
    file(WRITE ${CMAKE_BINARY_DIR}/compiler.sh "${COMPILER_SCRIPT_FILE}")
    execute_process(COMMAND ${CHMOD_PROGRAM} 755 ${CMAKE_BINARY_DIR}/compiler.sh)
    set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/compiler.sh)
  endif()
endmacro()

# finalize compiler script and write it
macro(finalize_compiler_script)
  if(ALLOW_CXXFLAGS_OVERWRITE AND (${CMAKE_GENERATOR} MATCHES ".*Unix Makefiles.*"))
    set( COMPILER_SCRIPT_FILE "#!${BASH}\nSED=${SED_PROGRAM}\nGREP=${GREP_PROGRAM}")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nCUT=${CUT_PROGRAM}\nENV=${ENV_PROGRAM}\nECHO=${ECHO_PROGRAM}")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n# store flags\nFLAGS=\"\$@\"")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nMAKE_EXECUTABLE_NEW=0\n")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nif [ \"\$CXXFLAGS\" == \"\" ]; then\n  # default CXX flags\n  CXXFLAGS=\"${DEFAULT_CXXFLAGS}\"\nfi\n")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nif [ \"\$EXTRA_CXXFLAGS\" != \"\" ]; then\n  # extra CXX flags\n  CXXFLAGS=\"$CXXFLAGS $EXTRA_CXXFLAGS\"\nfi\n")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nGRIDS=\nCONFIG_H=${CMAKE_BINARY_DIR}/config.h")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nif [ \"\$GRIDTYPE\" != \"\" ]; then")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  GRIDS=`\$GREP \"defined USED_[A-Z_]*_GRIDTYPE\" \$CONFIG_H | \$SED 's/\\(.*defined USED\\_\\)\\(.*\\)\\(\\_GRIDTYPE*\\)/\\2/g'`\nfi\n")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nOLDFLAGS=\$FLAGS\nFLAGS=")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nfor FLAG in \$OLDFLAGS; do")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  NEWFLAG=\$FLAG\n  VARNAME=`\$ECHO \$FLAG | \$GREP \"\\-D\" | \$SED 's/-D//g'`")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  for GRID in \$GRIDS; do\n    if [ \"\$VARNAME\" == \"\$GRID\" ]; then\n      NEWFLAG=\"-D\$GRIDTYPE\"\n      break\n    fi\n  done")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  VARNAME=`\$ECHO \$VARNAME | \$GREP \"=\" | \$CUT -d \"=\" -f 1`")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  if [ \"\$VARNAME\" != \"\" ]; then\n    VAR=`\$ENV | \$GREP \$VARNAME`\n    if [ \"\$VAR\" != \"\" ]; then")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n      # add variable from environment to flags list\n      NEWFLAG=\"-D\$VARNAME=\${!VARNAME}\"\n    fi\n  fi")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n  FLAGS=\"\$FLAGS \$NEWFLAG\"\ndone")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\n\$ECHO \"${DEFAULT_CXX_COMPILER} \$CXXFLAGS \$FLAGS\"")
    set( COMPILER_SCRIPT_FILE "${COMPILER_SCRIPT_FILE}\nexec ${DEFAULT_CXX_COMPILER} \$CXXFLAGS \$FLAGS")
    file(WRITE ${CMAKE_BINARY_DIR}/compiler.sh "${COMPILER_SCRIPT_FILE}")
  endif()
endmacro()
