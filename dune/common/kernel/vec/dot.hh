// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_VEC_DOT_HH
#define DUNE_COMMON_KERNEL_VEC_DOT_HH

#include <cstdint>
#include <utility>

#include <dune/common/promotiontraits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace vec {

      template<typename T1,
               typename T2,
               typename size_type,
               size_type alignment>
      typename PromotionTraits<T1,T2>::PromotedType
      dot(T1* DUNE_RESTRICT x,
          T2* DUNE_RESTRICT y,
          size_type n) DUNE_NOINLINE;


      template<typename T1,
               typename T2,
               typename size_type,
               size_type alignment>
      typename PromotionTraits<T1,T2>::PromotedType
      dot(T1* DUNE_RESTRICT x,
          T2* DUNE_RESTRICT y,
          size_type n)
      {
        typename PromotionTraits<T1,T2>::PromotedType res(0);
        DUNE_ASSUME_ALIGNED(x,T1,alignment);
        DUNE_ASSUME_ALIGNED(y,T2,alignment);
        for (size_type i = 0; i < n; ++i)
          res += x[i] * y[i];
        return std::move(res);
      }

#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,I,alignment)                               \
      template                                                          \
      typename PromotionTraits<T1,T2>::PromotedType                     \
      dot<T1,T2,I,alignment>(                                           \
        T1* DUNE_RESTRICT x,                                            \
        T2* DUNE_RESTRICT y,                                            \
        I n);

      DUNE_KERNEL_INSTANTIATE_SCALAR(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::size_t),ALIGNOF_SIZE_T)
      DUNE_KERNEL_INSTANTIATE_SCALAR(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::uint32_t),ALIGNOF_UINT32_T)
      DUNE_KERNEL_INSTANTIATE_SCALAR(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::size_t),ALIGNOF_FLOAT)
      DUNE_KERNEL_INSTANTIATE_SCALAR(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::uint32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      namespace blocked {

        template<typename T1,
                 typename T2,
                 typename size_type,
                 size_type alignment,
                 size_type block_size>
        typename PromotionTraits<T1,T2>::PromotedType
        dot(T1* DUNE_RESTRICT x,
            T2* DUNE_RESTRICT y,
            size_type n) DUNE_NOINLINE;

        template<typename T1,
                 typename T2,
                 typename size_type,
                 size_type alignment,
                 size_type block_size>
        typename PromotionTraits<T1,T2>::PromotedType
        dot(T1* DUNE_RESTRICT x,
            T2* DUNE_RESTRICT y,
            size_type n)
        {
          DUNE_ASSUME_ALIGNED(x,T1,alignment);
          DUNE_ASSUME_ALIGNED(y,T2,alignment);
          typename PromotionTraits<T1,T2>::PromotedType res(0);
          for (size_type b = 0; b != n; ++b)
            for (size_type i = 0; i < block_size; ++i)
              res += x[b*block_size + i] * y[b*block_size + i];
          return std::move(res);
        }


#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,I,alignment,block_size)                    \
        template                                                        \
        typename PromotionTraits<T1,T2>::PromotedType                   \
        dot<T1,T2,I,alignment,block_size>(                              \
          T1* DUNE_RESTRICT x,                                          \
          T2* DUNE_RESTRICT y,                                          \
          I n);

        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::size_t),ALIGNOF_SIZE_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::size_t),ALIGNOF_FLOAT)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::uint32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      } // namespace blocked
    } // namespace vec
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_VEC_DOT_HH
