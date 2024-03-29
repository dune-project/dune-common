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

dune_add_pybind11_module(NAME _common SOURCES _common.cc)
if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  set(FV_NUMS 0 1 2 3 4)
  foreach( FV_NUM ${FV_NUMS} )
    dune_add_pybind11_submodule(NAME registerfvector_${FV_NUM}
                                MODULE _common SOURCES registerfvector.cc
                                COMPILE_DEFINITIONS FV_NUM=${FV_NUM})
  endforeach()
endif()
set_property(TARGET _common PROPERTY LINK_LIBRARIES dunecommon APPEND)

if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
