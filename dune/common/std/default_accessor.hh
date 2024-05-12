// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_DEFAULT_ACCESSOR_HH
#define DUNE_COMMON_STD_DEFAULT_ACCESSOR_HH

#include <type_traits>

namespace Dune::Std {

/**
 * \brief A type for indexed access to elements of mdspan.
 * \ingroup CxxUtilities
 *
 * The `default_accessor` class template is the default `AccessorPolicy` used by `mdspan`
 * if no user-specified accessor policy is provided.
 *
 * \tparam Element  The element type. Shall be a complete object type that is neither an
 *                  abstract class type nor an array type. Otherwise, the program is ill-formed.
 */
template <class Element>
class default_accessor
{
public:
  using element_type = Element;
  using data_handle_type = element_type*;
  using reference = element_type&;
  using offset_policy = default_accessor;

public:
  // [mdspan.accessor.default.overview]
  /// \brief Default constructor
  constexpr default_accessor () noexcept = default;

  /// \brief Converting constructor from an accessor with different element type
  template <class OtherElement,
    std::enable_if_t<std::is_convertible_v<OtherElement(*)[], Element(*)[]>, int> = 0>
  constexpr default_accessor (default_accessor<OtherElement>) noexcept {}

  /// \brief Return a reference to the i'th element in the data range starting at `p`
  constexpr reference access (data_handle_type p, std::size_t i) const noexcept
  {
    return p[i];
  }

  /// \brief Return a data handle to the i'th element in the data range starting at `p`
  constexpr data_handle_type offset (data_handle_type p, std::size_t i) const noexcept
  {
    return p + i;
  }
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_DEFAULT_ACCESSOR_HH
