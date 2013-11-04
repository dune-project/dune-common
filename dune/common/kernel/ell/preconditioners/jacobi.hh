// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_ELL_PRECONDITIONERS_JACOBI_HH
#define DUNE_COMMON_KERNEL_ELL_PRECONDITIONERS_JACOBI_HH

#include <cstdint>
#include <cmath>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace ell {
      namespace preconditioners {

        namespace blocked {

          template<typename T1,
                   typename T2,
                   typename T3,
                   typename size_type,
                   size_type alignment,
                   size_type kernel_block_size>
          void jacobi(T1* DUNE_RESTRICT v_new,
                      const T1* DUNE_RESTRICT v_old,
                      const T2* DUNE_RESTRICT d,
                      const T3* DUNE_RESTRICT mat_data,
                      const size_type* DUNE_RESTRICT mat_col,
                      const size_type* DUNE_RESTRICT mat_block_offset,
                      T1* DUNE_RESTRICT diag,
                      T1* DUNE_RESTRICT rhs,
                      size_type n,
                      size_type kernel_offset,
                      size_type size,
                      const T1 relaxation_factor) DUNE_NOINLINE;

          template<typename T1,
                   typename T2,
                   typename T3,
                   typename size_type,
                   size_type alignment,
                   size_type kernel_block_size>
          void jacobi(T1* DUNE_RESTRICT v_new,
                      const T1* DUNE_RESTRICT v_old,
                      const T2* DUNE_RESTRICT d,
                      const T3* DUNE_RESTRICT mat_data,
                      const size_type* DUNE_RESTRICT mat_col,
                      const size_type* DUNE_RESTRICT mat_block_offset,
                      T1* DUNE_RESTRICT diag,
                      T1* DUNE_RESTRICT rhs,
                      size_type n,
                      size_type kernel_offset,
                      size_type size,
                      const T1 relaxation_factor)
          {
            DUNE_ASSUME_ALIGNED(v_new,T1,alignment);
            DUNE_ASSUME_ALIGNED(v_old,T1,alignment);
            DUNE_ASSUME_ALIGNED(d,T2,alignment);
            DUNE_ASSUME_ALIGNED(mat_data,T3,alignment);
            DUNE_ASSUME_ALIGNED(mat_col,size_type,alignment);
            DUNE_ASSUME_ALIGNED(mat_block_offset,size_type,alignment/kernel_block_size);
            DUNE_ASSUME_ALIGNED(diag,T1,alignment);
            DUNE_ASSUME_ALIGNED(rhs,T1,alignment);
            size_type offset = 0;
            for (size_type block = 0; block < n; ++block)
              {
                size_type cols = (mat_block_offset[block+1] - mat_block_offset[block]) >> Memory::block_size_log2<kernel_block_size>::value;

                // extract data for current block from d
                for (int i = 0; i < kernel_block_size; ++i)
                  rhs[i] = d[block*kernel_block_size + i];

                // clear out diagonal, but set padded diagonals to 1 to avoid division by zero
                for (int i = 0; i < kernel_block_size; ++i)
                  diag[i] = kernel_offset + block * kernel_block_size + i < size ? 0.0 : 1.0;

                // calculate rhs = d - (L+U) * v_old
                for (int j = 0; j < cols; ++j)
                  {
                    // extract data for current block (in column direction) from v_old and
                    // apply mask zeroing out data on diagonal block
                    for (int i = 0; i < kernel_block_size; ++i)
                      {
                        // do mmv operation off-diagonal
                        rhs[i] -= mat_data[(offset + j) * kernel_block_size + i] * (kernel_offset + block * kernel_block_size + i != mat_col[offset*kernel_block_size + kernel_block_size*j+i]) * v_old[mat_col[offset*kernel_block_size + kernel_block_size*j+i]];
                        // save diagonal value
                        diag[i] += (kernel_offset + block * kernel_block_size + i != mat_col[offset*kernel_block_size + kernel_block_size*j+i]) * v_old[mat_col[offset*kernel_block_size + kernel_block_size*j+i]];
                      }
                  }


                // now solve D * v_new = rhs

                for (int i = 0; i < kernel_block_size; ++i)
                  v_new[i] = rhs[i] / diag[i];

                offset += cols;
              }
          }


  #ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,T3,I,alignment,kernel_block_size)          \
          template                                                      \
          void jacobi<T1,T2,T3,I,alignment,kernel_block_size>(          \
            T1* DUNE_RESTRICT v_new,                                    \
            const T1* DUNE_RESTRICT v_old,                              \
            const T2* DUNE_RESTRICT d,                                  \
            const T3* DUNE_RESTRICT mat_data,                           \
            const I*  DUNE_RESTRICT mat_col,                            \
            const I*  DUNE_RESTRICT mat_block_offset,                   \
            T1* DUNE_RESTRICT diag,                                     \
            T1* DUNE_RESTRICT rhs,                                      \
            I n,                                                        \
            I kernel_offset,                                            \
            I size,                                                     \
            T1 relaxation_factor                                        \
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
      } // namespace preconditioners
    } // namespace ell
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_ELL_PRECONDITIONERS_JACOBI_HH
