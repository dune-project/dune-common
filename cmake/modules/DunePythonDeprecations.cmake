# This file only contains the deprecation messages from the overhaul
# of the Python CMake integration. After the deprecation period, this
# file can simply be deleted (and its include in DunePythonCommonMacros.cmake removed)

if(DUNE_PYTHON_INSTALL_EDITABLE)
  message(WARNING
"    The DUNE_PYTHON_INSTALL_EDITABLE variable is deprecated.
     Installations into the Dune-internal virtualenv are always
     editable. If you want to enable editable installation into
     your system environment (e.g. because it is a virtual environment),
     you can do so with `-DDUNE_PYTHON_ADDITIONAL_PIP_PARAMS=--editable`
")
endif()

if(DUNE_PYTHON_VIRTUALENV_SETUP)
  message(WARNING
"    The DUNE_PYTHON_VIRTUALENV_SETUP variable is deprecated, as it's
     behaviour is now enabled by default.
")
endif()

function(dune_python_require_virtualenv_setup)
  message(WARNING
"    The dune_python_require_virtualenv_setup function is deprecated, as the
     internal virtual environment is now enabled by default.
")
endfunction()
