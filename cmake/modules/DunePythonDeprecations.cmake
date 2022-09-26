# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# This file only contains the deprecation messages from the overhaul
# of the Python CMake integration. After the deprecation period, this
# file can simply be deleted (and its include in DunePythonCommonMacros.cmake removed)

if(DUNE_PYTHON_INSTALL_EDITABLE)
  message(WARNING
"    The DUNE_PYTHON_INSTALL_EDITABLE variable is deprecated.
     Installations into the Dune-internal virtualenv (or active external
     virtualenv are always editable. An editable installation e.g. into
     your system environment is not supported at the moment since an
     installation should not depend on the existence of a build directory.
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
