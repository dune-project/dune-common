# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_python_targets(typeregistry
  __init__
)
dune_add_pybind11_module(NAME _typeregistry)
if (SKBUILD)
  install(TARGETS _typeregistry LIBRARY DESTINATION python/dune/typeregistry)
endif()
