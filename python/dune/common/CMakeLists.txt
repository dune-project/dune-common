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
  pickle                # deprecated 2.8
  project
  utility
)
dune_add_pybind11_module(NAME _common)
set_property(TARGET _common PROPERTY LINK_LIBRARIES dunecommon APPEND)
if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
