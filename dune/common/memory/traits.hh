// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_MEMORY_TRAITS_HH
#define DUNE_COMMON_MEMORY_TRAITS_HH

#include <dune/common/typetraits.hh>

namespace Dune {
  namespace Memory {

    template<typename A1, typename A2>
    struct allocators_are_interoperable
      : public integral_constant<bool,
                                 is_same<
                                   typename A1::template rebind<void>::other,
                                   typename A2::template rebind<void>::other
                                   >::value
                                 >
    {};

  } // namespace Memory
} //namespace Dune

#endif // DUNE_COMMON_MEMORY_TRAITS_HH
