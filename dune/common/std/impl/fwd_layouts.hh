// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_IMPL_FWD_LAYOUTS_HH
#define DUNE_COMMON_STD_IMPL_FWD_LAYOUTS_HH

namespace Dune::Std {

/**
 * \brief A layout where the leftmost extent has stride 1.
 * \ingroup CxxUtilities
 *
 * For two-dimensional tensors this corresponds to column-major indexing.
 **/
struct layout_left
{
  template <class Extents>
  class mapping;
};

/**
 * \brief A layout where the rightmost extent has stride 1, and strides
 *        increase right-to-left as the product of extents.
 * \ingroup CxxUtilities
 *
 * For two-dimensional tensors this corresponds to row-major indexing.
 **/
struct layout_right
{
  template <class Extents>
  class mapping;
};

/**
 * \brief A layout mapping where the strides are user-defined.
 * \ingroup CxxUtilities
 **/
struct layout_stride
{
  template <class Extents>
  class mapping;
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_IMPL_FWD_LAYOUTS_HH
