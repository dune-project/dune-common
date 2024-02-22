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

set(COMMON_SOURCES _common.cc)
if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  set(COMMON_SOURCES ${COMMON_SOURCES} registerfvector_a.cc  registerfvector_b.cc registerfvector_c.cc  registerfvector_d.cc)
endif()

dune_add_pybind11_module(NAME _common SOURCES ${COMMON_SOURCES})
set_property(TARGET _common PROPERTY LINK_LIBRARIES dunecommon APPEND)
if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
