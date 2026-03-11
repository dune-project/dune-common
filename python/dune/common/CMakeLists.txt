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
target_link_libraries(_common PUBLIC Dune::Common)

if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  foreach(FV_NUM RANGE 0 4)
    add_library(registerfvector_${FV_NUM} OBJECT registerfvector.cc)
    target_compile_definitions(registerfvector_${FV_NUM} PRIVATE FV_NUM=${FV_NUM})
    target_link_libraries(registerfvector_${FV_NUM} PRIVATE Dune::Common)

    # Register the object in the pybind11 module _common
    target_link_libraries(_common PRIVATE registerfvector_${FV_NUM})
  endforeach()
endif()

if(SKBUILD)
  install(TARGETS _common LIBRARY DESTINATION python/dune/common)
endif()
