// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_MEMORY_HH
#define DUNE_COMMON_STD_MEMORY_HH

#include <memory>
#include <type_traits>

#include <dune/common/typeutilities.hh>

namespace Dune::Std {

#if __cpp_lib_to_address >= 201711L

using std::to_address;

#else

namespace Impl {

template <class T>
constexpr T* toAddressImpl (T* p, Dune::PriorityTag<2>) noexcept
{
  static_assert(!std::is_function_v<T>);
  return p;
}

template <class T>
constexpr auto toAddressImpl (const T& p, Dune::PriorityTag<1>) noexcept
  -> decltype(std::pointer_traits<T>::to_address(p))
{
  return std::pointer_traits<T>::to_address(p);
}

template <class T>
constexpr auto toAddressImpl (const T& p, Dune::PriorityTag<0>) noexcept
{
  return toAddressImpl(p.operator->(), Dune::PriorityTag<3>{});
}

} // end namespace Impl

/// \brief Obtain the address represented by `p` without forming a reference to the object pointed to by `p`.
template <class T>
constexpr auto to_address(T&& p) noexcept
{
  return Impl::toAddressImpl(std::forward<T>(p), Dune::PriorityTag<3>{});
}

#endif

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_MEMORY_HH
