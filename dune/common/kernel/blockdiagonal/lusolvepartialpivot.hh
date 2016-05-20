// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUSOLVEPARTIALPIVOT_HH
#define DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUSOLVEPARTIALPIVOT_HH

#include <cstdint>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace block_diagonal {

      namespace blocked {

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void lu_solve_partial_pivot(T1* DUNE_RESTRICT v,
                                    const T2* DUNE_RESTRICT d,
                                    const T3* DUNE_RESTRICT mat_data,
                                    T1* DUNE_RESTRICT y,
                                    const size_type* DUNE_RESTRICT permutation,
                                    size_type n,
                                    size_type block_size,
                                    const T1 relaxation_factor) DUNE_NOINLINE;

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void lu_solve_partial_pivot(T1* DUNE_RESTRICT v,
                                    const T2* DUNE_RESTRICT d,
                                    const T3* DUNE_RESTRICT mat_data,
                                    T1* DUNE_RESTRICT y,
                                    const size_type* DUNE_RESTRICT permutation,
                                    size_type n,
                                    size_type block_size,
                                    const T1 relaxation_factor)
        {
          DUNE_ASSUME_ALIGNED(v,T1,alignment);
          DUNE_ASSUME_ALIGNED(d,T2,alignment);
          DUNE_ASSUME_ALIGNED(mat_data,T3,alignment);
          DUNE_ASSUME_ALIGNED(y,T1,alignment);
          DUNE_ASSUME_ALIGNED(permutation,const size_type,alignment);
          for (size_type block = 0; block < n; ++block)
            {
              // forward substitution
              for (size_type ii = 0; ii < block_size; ++ii)
                {

                  // do first column separately to avoid having to zero out y1 and avoid branch by masking with (ii > 0)
                  for (size_type i = 0; i < kernel_block_size; ++i)
                    y[ii * kernel_block_size + i] = (ii > 0) * (-mat_data[((block * block_size + ii) * block_size) * kernel_block_size + i] * y[i]);

                  for (size_type jj = 1; jj < ii; ++jj)
                    {
                      for (size_type i = 0; i < kernel_block_size; ++i)
                        y[ii * kernel_block_size + i] -= mat_data[((block * block_size + ii) * block_size + jj) * kernel_block_size + i] * y[jj * kernel_block_size + i];
                    }
                  for (size_type i = 0; i < kernel_block_size; ++i)
                    y[ii * kernel_block_size + i] += d[block * block_size * kernel_block_size + i * block_size + permutation[block * block_size * kernel_block_size + i * block_size + ii]];
                }

              // backward substitution
              for (size_type ii = block_size - 1; ii >= 0; --ii)
                {
                  for (size_type jj = ii + 1; jj < block_size - 1; ++jj)
                    {
                      for (size_type i = 0; i < kernel_block_size; ++i)
                        y[ii * kernel_block_size + i] -= mat_data[((block * block_size + ii) * block_size + jj) * kernel_block_size + i] * y[jj * kernel_block_size + i];
                    }

                  // calculate relaxation and update result
                  //T1 retention_factor = T1(1.0) - relaxation_factor;
                  for (size_type i = 0; i < kernel_block_size; ++i)
                    v[block * block_size * kernel_block_size + i * block_size + permutation[block * block_size * kernel_block_size + i * block_size + ii]] += relaxation_factor * y[ii * kernel_block_size + i];
                    //v[(block * block_size + i) * kernel_block_size + permutation[(block * block_size + i) * kernel_block_size + ii]] = relaxation_factor * y2[ii * kernel_block_size + i] + retention_factor * v[(block * block_size + i) * kernel_block_size + permutation[(block * block_size + i) * kernel_block_size + ii]];
                }

            }
        }


#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,T3,I,alignment,kernel_block_size)          \
        template                                                        \
        void lu_solve_partial_pivot<T1,T2,T3,I,alignment,kernel_block_size>( \
          T1* DUNE_RESTRICT v,                                          \
          const T2* DUNE_RESTRICT d,                                    \
          const T3* DUNE_RESTRICT mat_data,                             \
          T1* DUNE_RESTRICT y,                                          \
          const I*  DUNE_RESTRICT permutation,                          \
          I n,                                                          \
          I block_size,                                                 \
          T1 relaxation_factor                                          \
          );

        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,double,std::size_t),ALIGNOF_SIZE_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,double,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,double,std::int32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,float,std::size_t),ALIGNOF_FLOAT)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,float,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,float,std::int32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      } // namespace blocked
    } // namespace block_diagonal
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUSOLVEPARTIALPIVOT_HH
