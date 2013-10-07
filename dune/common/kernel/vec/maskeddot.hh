// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_VEC_MASKEDDOT_HH
#define DUNE_COMMON_KERNEL_VEC_MASKEDDOT_HH

#include <cstdint>
#include <utility>

#include <dune/common/promotiontraits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace vec {

      // predicate is moved to the end of the template argument list to avoid having
      // to spell out the type of a lambda (awkward because only doable with decltype)

      template<typename T1,
               typename T2,
               typename T3,
               typename size_type,
               size_type alignment,
               typename Predicate>
      typename PromotionTraits<T1,T2>::PromotedType
      masked_dot(T1* DUNE_RESTRICT x,
                 T2* DUNE_RESTRICT y,
                 T3* DUNE_RESTRICT mask,
                 Predicate predicate,
                 size_type n) DUNE_NOINLINE;


      template<typename T1,
               typename T2,
               typename T3,
               typename size_type,
               size_type alignment,
               typename Predicate>
      typename PromotionTraits<T1,T2>::PromotedType
      masked_dot(T1* DUNE_RESTRICT x,
                 T2* DUNE_RESTRICT y,
                 T3* DUNE_RESTRICT mask,
                 Predicate predicate,
                 size_type n)
      {
        typename PromotionTraits<T1,T2>::PromotedType res(0);
        DUNE_ASSUME_ALIGNED(x,T1,alignment);
        DUNE_ASSUME_ALIGNED(y,T2,alignment);
        DUNE_ASSUME_ALIGNED(mask,T3,alignment);
        for (size_type i = 0; i < n; ++i)
          res += predicate(x[i],y[i],mask[i]) * (x[i] * y[i]);
        return std::move(res);
      }

      namespace blocked {

        // predicate is moved to the end of the template argument list to avoid having
        // to spell out the type of a lambda (awkward because only doable with decltype)

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type block_size,
                 typename Predicate>
        typename PromotionTraits<T1,T2>::PromotedType
        masked_dot(T1* DUNE_RESTRICT x,
                   T2* DUNE_RESTRICT y,
                   T3* DUNE_RESTRICT mask,
                   Predicate predicate,
                   size_type n) DUNE_NOINLINE;

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type block_size,
                 typename Predicate>
        typename PromotionTraits<T1,T2>::PromotedType
        masked_dot(T1* DUNE_RESTRICT x,
                   T2* DUNE_RESTRICT y,
                   T3* DUNE_RESTRICT mask,
                   Predicate predicate,
                   size_type n)
        {
          DUNE_ASSUME_ALIGNED(x,T1,alignment);
          DUNE_ASSUME_ALIGNED(y,T2,alignment);
          typename PromotionTraits<T1,T2>::PromotedType res(0);
          for (size_type b = 0; b != n; ++b)
            for (size_type i = 0; i < block_size; ++i)
              res += predicate(x[b*block_size + i],y[b*block_size + i],mask[b*block_size + i]) * (x[b*block_size + i] * y[b*block_size + i]);
          return std::move(res);
        }

      } // namespace blocked
    } // namespace vec
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_VEC_MASKEDDOT_HH
