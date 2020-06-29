# this option enables the build of Python bindings for DUNE modules
option(DUNE_ENABLE_PYTHONBINDINGS "Enable Python bindings for DUNE" OFF)

if( DUNE_ENABLE_PYTHONBINDINGS )
  if(NOT PYTHONINTERP_FOUND)
    message(FATAL_ERROR "Module dune-python requires a Python interpreter")
  endif()
  if(NOT PYTHONLIBS_FOUND)
    message(FATAL_ERROR "Found a Python interpreter but module dune-python also requires the Python libraries (e.g. a python-dev package)")
  endif()

  include_directories("${PYTHON_INCLUDE_DIRS}")

  function(add_python_targets base)
    include(DuneSymlinkOrCopy)
    if( "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}" )
      message(WARNING "Source and binary dir are the same, skipping symlink!")
    else()
      foreach(file ${ARGN})
        dune_symlink_to_source_files(FILES ${file}.py)
      endforeach()
    endif()
  endfunction()

  include(DuneAddPybind11Module)
endif()
