# This module provided functions to implement constraints on the major version of the python interpreter.
# With Python2 support being dropped from Dune these are not needed anymore.

if(DUNE_PYTHON_FORCE_PYTHON2 OR DUNE_PYTHON_FORCE_PYTHON3 OR DUNE_PYTHON_FORCE_PYTHON_VERSION)
  message(WARNING "Python 2 support has been dropped from Dune. The variables DUNE_PYTHON_FORCE_PYTHON2, DUNE_PYTHON_FORCE_PYTHON3 and DUNE_PYTHON_FORCE_PYTHON_VERSION variables are not doing anything anymore.")
endif()

function(dune_python_require_version)
  message(WARNING "Dune has dropped Python2 support. The function dune_python_require_version is now no-op, is deprecated, and will be removed after Dune 2.7.")
endfunction()

function(dune_python_force_version)
  message(WARNING "Dune has dropped Python2 support. The function dune_python_force_version is now no-op, is deprecated, and will be removed after Dune 2.7.")
endfunction()
