// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_IMPL_CONTAINER_CONSTRUCTION_TRAITS_HH
#define DUNE_COMMON_STD_IMPL_CONTAINER_CONSTRUCTION_TRAITS_HH

#include <array>
#include <cassert>
#include <cstddef>

namespace Dune::Std {
namespace Impl {

/**
 * \brief A traits class to specialize on how to construct a given container type.
 *
 * A specialization for std::array types is provided.
 */
template <class Container>
struct ContainerConstructionTraits
{
  using container_type = Container;
  using value_type = typename container_type::value_type;

  static constexpr container_type construct (std::size_t size)
  {
    return container_type(size);
  }

  static constexpr container_type construct (std::size_t size, value_type value)
  {
    return container_type(size, value);
  }
};

template <class Value, std::size_t N>
struct ContainerConstructionTraits<std::array<Value,N>>
{
  using container_type = std::array<Value,N>;
  using value_type = typename container_type::value_type;

  static constexpr container_type construct ([[maybe_unused]] std::size_t size)
  {
    assert(size <= N);
    return container_type{};
  }

  static constexpr container_type construct ([[maybe_unused]] std::size_t size, value_type value)
  {
    assert(size <= N);
    container_type c{};
    for (auto& c_i : c)
      c_i = value;
    return c;
  }
};

} // end namespace Impl
} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_IMPL_CONTAINER_CONSTRUCTION_TRAITS_HH
