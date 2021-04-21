# this option enables the build of Python bindings for DUNE modules
option(DUNE_ENABLE_PYTHONBINDINGS "Enable Python bindings for DUNE" OFF)

if( DUNE_ENABLE_PYTHONBINDINGS )
  if(NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR "Python bindings require a Python 3 interpreter")
  endif()
  if(NOT Python3_INCLUDE_DIRS)
    message(FATAL_ERROR "Found a Python interpreter but the Python bindings also requires the Python libraries (a package named like python-dev package or python3-devel)")
  endif()

  include_directories("${Python3_INCLUDE_DIRS}")

  function(add_python_targets base)
    include(DuneSymlinkOrCopy)
    if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
      message(WARNING "Source and binary dir are the same, skipping symlink!")
    else()
      foreach(file ${ARGN})
        dune_symlink_to_source_files(FILES ${file}.py)
      endforeach()
    endif()
  endfunction()

  include(DuneAddPybind11Module)

  # Add a custom command that triggers the configuration of dune-py
  add_custom_command(TARGET install_python POST_BUILD
                     COMMAND ${Python3_EXECUTABLE} -m dune configure
                     )

endif()
