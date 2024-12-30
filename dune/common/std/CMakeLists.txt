# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

install(FILES
  algorithm.hh
  compare.hh
  default_accessor.hh
  extents.hh
  functional.hh
  iterator.hh
  layout_left.hh
  layout_right.hh
  layout_stride.hh
  mdarray.hh
  mdspan.hh
  memory.hh
  no_unique_address.hh
  span.hh
  type_traits.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/std)

add_subdirectory(impl)
add_subdirectory(test)