// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_UTILITY_HH
#define DUNE_COMMON_KERNEL_UTILITY_HH

#include <dune/common/typetraits.hh>


#define DUNE_RESTRICT __restrict__
#define DUNE_NOINLINE __attribute__((noinline))

#if defined __clang__
#define DUNE_ASSUME_ALIGNED(x,T,alignment)
#else
#define DUNE_ASSUME_ALIGNED(x,T,alignment) x = static_cast<T*>(__builtin_assume_aligned(x,alignment))
#endif

namespace Dune {
  namespace Kernel {

    template<typename A1, typename A2>
    struct allocators_are_interoperable
      : public integral_constant<bool,
                                 is_same<
                                   typename A1::template rebind<void>::other,
                                   typename A2::template rebind<void>::other
                                   >::value
                                 >
    {};

  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_UTILITY_HH
