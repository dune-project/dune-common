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
    add_library(registerfvector_${FV_NUM} OBJECT registerfvector.cc)
    target_compile_definitions(registerfvector_${FV_NUM} PRIVATE FV_NUM=${FV_NUM})
    # minimal c++ standard required
    target_compile_features(registerfvector_${FV_NUM} PUBLIC cxx_std_17)
    target_link_libraries(_common PUBLIC registerfvector_${FV_NUM})
  endforeach()
endif()
set_property(TARGET _common PROPERTY LINK_LIBRARIES dunecommon APPEND)

if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
