// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUDECOMPOSITIONPARTIALPIVOT_HH
#define DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUDECOMPOSITIONPARTIALPIVOT_HH

#include <cstdint>
#include <cmath>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace block_diagonal {

      namespace blocked {

        template<typename T1,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void lu_decomposition_partial_pivot(T1* DUNE_RESTRICT mat_in,
                                            T1* DUNE_RESTRICT mat_out,
                                            size_type* DUNE_RESTRICT permutation,
                                            size_type n,
                                            size_type block_size) DUNE_NOINLINE;

        template<typename T1,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void lu_decomposition_partial_pivot(T1* DUNE_RESTRICT mat_in,
                                            T1* DUNE_RESTRICT mat_out,
                                            size_type* DUNE_RESTRICT permutation,
                                            size_type n,
                                            size_type block_size)
        {
          DUNE_ASSUME_ALIGNED(mat_in,T1,alignment);
          DUNE_ASSUME_ALIGNED(mat_out,T1,alignment);
          DUNE_ASSUME_ALIGNED(permutation,size_type,alignment);
          for (size_type block = 0; block < n; ++block)
            {
              for (int ii = 0; ii < block_size; ++ii)
                {
                  // find largest column entry
                  size_type offset[kernel_block_size];
                  for (int i = 0; i < kernel_block_size; ++i)
                    offset[i] = ii;
                  size_type is_larger[kernel_block_size] = {0};
                  T1 maxval[kernel_block_size] = {0.0};
                  T1 val[kernel_block_size] = {0.0};
                  for (int k = ii; k < block_size; ++k)
                    {
                      for (int i = 0; i < kernel_block_size; ++i)
                        val[i] = std::abs(mat_in[((block * block_size + k) * block_size + ii) * kernel_block_size + i]);
                      for (int i = 0; i < kernel_block_size; ++i)
                        {
                          offset[i] = is_larger[i] * k + (1-is_larger[i]) * offset[i];
                          maxval[i] = is_larger[i] * val[i] + (1-is_larger[i]) * maxval[i];
                        }
                    }

                  for (int i = 0; i < kernel_block_size; ++i)
                    permutation[(block * block_size + ii) * kernel_block_size + i] = offset[i];

                  for (int jj = 0; jj < block_size; ++jj)
                    for (int i = 0; i < kernel_block_size; ++i)
                      {
                        val[i] = mat_in[((block * block_size + ii) * block_size + jj) * kernel_block_size + i];
                        mat_out[((block * block_size + ii) * block_size + jj) * kernel_block_size + i] = mat_in[((block * block_size + offset[i]) * block_size + jj) * kernel_block_size + i];
                        mat_in[((block * block_size + offset[i]) * block_size + jj) * kernel_block_size + i] = val[i];
                      }

                  for (int k = ii + 1; k < block_size; ++k)
                    {
                      // divide remainder of column by diagonal entry
                      for (int i = 0; i < kernel_block_size; ++i)
                        mat_out[((block * block_size + k) * block_size + ii) * kernel_block_size + i] /= mat_in[((block * block_size + ii) * block_size + ii) * kernel_block_size + i];

                      // transform bottom right corner matrix
                      for (int jj = ii + 1; jj < block_size; ++jj)
                        for (int i = 0; i < kernel_block_size; ++i)
                          mat_out[((block * block_size + k) * block_size + jj) * kernel_block_size + i] -= mat_in[((block * block_size + k) * block_size + ii) * kernel_block_size + i] * mat_in[((block * block_size + ii) * block_size + jj) * kernel_block_size + i];
                    }
                }
            }
        }


#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,I,alignment,kernel_block_size)                \
        template                                                        \
        void lu_decomposition_partial_pivot<T1,I,alignment,kernel_block_size>(        \
          T1* DUNE_RESTRICT mat_in,                                     \
          T1* DUNE_RESTRICT mat_out,                                    \
          I*  DUNE_RESTRICT permutation,                                \
          I n,                                                          \
          I block_size                                                  \
          );

        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,std::size_t),ALIGNOF_SIZE_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,std::int32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,std::size_t),ALIGNOF_FLOAT)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,std::int32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      } // namespace blocked
    } // namespace block_diagonal
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_BLOCKDIAGONAL_LUDECOMPOSITIONPARTIALPIVOT_HH
