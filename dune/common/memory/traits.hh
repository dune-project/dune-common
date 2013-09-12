// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_MEMORY_TRAITS_HH
#define DUNE_COMMON_MEMORY_TRAITS_HH

#include <dune/common/typetraits.hh>
#include <dune/common/static_assert.hh>

namespace Dune {
  namespace Memory {

    namespace impl {

      template<typename... A>
      struct allocators_are_interoperable;

      template<typename A>
      struct allocators_are_interoperable<A>
        : public true_type
      {};

      template<typename A1, typename A2, typename... A>
      struct allocators_are_interoperable<A1,A2,A...>
      {

        static const bool tail_is_interoperable =
          allocators_are_interoperable<A2,A...>::value;

        static const bool head_is_interoperable = is_same<
          typename A1::template rebind<void>::other,
          typename A2::template rebind<void>::other
          >::value;

        static const bool value = head_is_interoperable && tail_is_interoperable;

      };

    } // namespace impl

    // repackage into integral_constant
    template<typename... A>
    struct allocators_are_interoperable
      : public integral_constant<bool,
                                 impl::allocators_are_interoperable<
                                   A...
                                   >::value
                                 >
    {};


    template<std::size_t i, std::size_t limit = 8>
    struct block_size_log2
      : public conditional<(i == (1<<limit)),
        integral_constant<std::size_t,limit>,
        block_size_log2<i,limit-1>
        >::type
    {};

    template<std::size_t i>
    struct block_size_log2<i,0>
      : public integral_constant<std::size_t,0>
    {
      dune_static_assert(i == 1, "Unsupported block size! Has to be a power of two up to an implementation-defined maximum.");
    };

  } // namespace Memory
} //namespace Dune

#endif // DUNE_COMMON_MEMORY_TRAITS_HH
