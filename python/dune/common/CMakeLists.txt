# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_python_targets(common
  __init__
  checkconfiguration
  compatibility
  deprecated            # deprecated 2.8
  hashit
  locking
  module
  pickle
  project
  utility
)
dune_add_pybind11_module(NAME _common)
target_link_libraries(_common PUBLIC Dune::Common)
if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
