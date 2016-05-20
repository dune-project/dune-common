// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_BELL_PRECONDITIONERS_JACOBINOLUPIVOT_HH
#define DUNE_COMMON_KERNEL_BELL_PRECONDITIONERS_JACOBINOLUPIVOT_HH

#include <cstdint>
#include <cmath>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace bell {
      namespace preconditioners {

        namespace blocked {

          template<typename T1,
                   typename T2,
                   typename T3,
                   typename size_type,
                   size_type alignment,
                   size_type kernel_block_size>
          void jacobi_no_lu_pivot(T1* DUNE_RESTRICT v_new,
                                  const T1* DUNE_RESTRICT v_old,
                                  const T2* DUNE_RESTRICT d,
                                  const T3* DUNE_RESTRICT diag_mat_data,
                                  const T3* DUNE_RESTRICT mat_data,
                                  const size_type* DUNE_RESTRICT mat_col,
                                  const size_type* DUNE_RESTRICT mat_block_offset,
                                  T1* DUNE_RESTRICT y,
                                  T1* DUNE_RESTRICT rhs,
                                  size_type n,
                                  size_type block_dim,
                                  size_type kernel_offset,
                                  const T1 relaxation_factor) DUNE_NOINLINE;

          template<typename T1,
                   typename T2,
                   typename T3,
                   typename size_type,
                   size_type alignment,
                   size_type kernel_block_size>
          void jacobi_no_lu_pivot(T1* DUNE_RESTRICT v_new,
                                  const T1* DUNE_RESTRICT v_old,
                                  const T2* DUNE_RESTRICT d,
                                  const T3* DUNE_RESTRICT diag_mat_data,
                                  const T3* DUNE_RESTRICT mat_data,
                                  const size_type* DUNE_RESTRICT mat_col,
                                  const size_type* DUNE_RESTRICT mat_block_offset,
                                  T1* DUNE_RESTRICT y,
                                  T1* DUNE_RESTRICT rhs,
                                  size_type n,
                                  size_type block_dim,
                                  size_type kernel_offset,
                                  const T1 relaxation_factor)
          {
            DUNE_ASSUME_ALIGNED(v_new,T1,alignment);
            DUNE_ASSUME_ALIGNED(v_old,T1,alignment);
            DUNE_ASSUME_ALIGNED(d,T2,alignment);
            DUNE_ASSUME_ALIGNED(diag_mat_data,T3,alignment);
            DUNE_ASSUME_ALIGNED(mat_data,T3,alignment);
            DUNE_ASSUME_ALIGNED(mat_col,size_type,alignment);
            DUNE_ASSUME_ALIGNED(mat_block_offset,size_type,alignment/kernel_block_size);
            DUNE_ASSUME_ALIGNED(y,T1,alignment);
            DUNE_ASSUME_ALIGNED(rhs,T1,alignment);
            size_type offset = 0;
            for (size_type block = 0; block < n; ++block)
              {
                size_type cols = (mat_block_offset[block+1] - mat_block_offset[block]) >> Memory::block_size_log2<kernel_block_size>::value;

                // extract data for current block from d
                for (size_type ii = 0; ii < block_dim; ++ii)
                  for (size_type i = 0; i < kernel_block_size; ++i)
                    rhs[ii * kernel_block_size + i] = d[block*kernel_block_size*block_dim + ii + i*block_dim];

                // calculate rhs = d - (L+U) * v_old
                for (size_type j = 0; j < cols; ++j)
                  {
                    // extract data for current block (in column direction) from v_old and
                    // apply mask zeroing out data on diagonal block
                    for (size_type jj = 0; jj < block_dim; ++jj)
                      for (size_type i = 0; i < kernel_block_size; ++i)
                        y[jj * kernel_block_size + i] = (kernel_offset + block * kernel_block_size + i != mat_col[offset*kernel_block_size + kernel_block_size*j+i]) * v_old[mat_col[offset*kernel_block_size + kernel_block_size*j+i] * block_dim + jj];
                    // do mmv operation
                    for (size_type ii = 0; ii < block_dim; ++ii)
                      {
                        for (size_type jj = 0; jj < block_dim; ++jj)
                          for (size_type i = 0; i < kernel_block_size; ++i)
                            rhs[ii * kernel_block_size + i] -= mat_data[(((offset + j)*block_dim + ii) * block_dim + jj) * kernel_block_size + i] * y[jj * kernel_block_size + i];
                      }
                  }


                // now solve D * v_new = rhs

                // forward substitution
                for (size_type ii = 0; ii < block_dim; ++ii)
                  {

                    // do first column separately to avoid having to zero out y1 and avoid branch by masking with (ii > 0)
                    // this makes sure y will always be initialized to a known value
                    for (size_type i = 0; i < kernel_block_size; ++i)
                      y[ii * kernel_block_size + i] = (ii > 0) * (-diag_mat_data[((block * block_dim + ii) * block_dim) * kernel_block_size + i] * y[i]);

                    for (size_type jj = 1; jj < ii; ++jj)
                      {
                        for (size_type i = 0; i < kernel_block_size; ++i)
                          y[ii * kernel_block_size + i] -= diag_mat_data[((block * block_dim + ii) * block_dim + jj) * kernel_block_size + i] * y[jj * kernel_block_size + i];
                      }
                    for (size_type i = 0; i < kernel_block_size; ++i)
                      y[ii * kernel_block_size + i] += rhs[ii * kernel_block_size + i];
                  }

                // backward substitution
                for (size_type ii = block_dim - 1; ii >= 0; --ii)
                  {
                    for (size_type jj = ii + 1; jj < block_dim; ++jj)
                      {
                        for (size_type i = 0; i < kernel_block_size; ++i)
                          y[ii * kernel_block_size + i] -= diag_mat_data[((block * block_dim + ii) * block_dim + jj) * kernel_block_size + i] * y[jj * kernel_block_size + i];
                      }

                    for (size_type i = 0; i < kernel_block_size; ++i)
                      y[ii * kernel_block_size + i] /= diag_mat_data[((block * block_dim + ii) * block_dim + ii) * kernel_block_size + i];

                    // update output vector
                    for (size_type i = 0; i < kernel_block_size; ++i)
                      v_new[block * block_dim * kernel_block_size + i * block_dim + ii] = y[ii * kernel_block_size + i];
                  }
                offset += cols;
              }
          }


  #ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,T3,I,alignment,kernel_block_size)          \
          template                                                      \
          void jacobi_no_lu_pivot<T1,T2,T3,I,alignment,kernel_block_size>( \
            T1* DUNE_RESTRICT v_new,                                    \
            const T1* DUNE_RESTRICT v_old,                              \
            const T2* DUNE_RESTRICT d,                                  \
            const T3* DUNE_RESTRICT diag_mat_data,                      \
            const T3* DUNE_RESTRICT mat_data,                           \
            const I*  DUNE_RESTRICT mat_col,                            \
            const I*  DUNE_RESTRICT mat_block_offset,                   \
            T1* DUNE_RESTRICT y,                                        \
            T1* DUNE_RESTRICT rhs,                                      \
            I n,                                                        \
            I block_dim,                                                \
            I kernel_offset,                                            \
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
    } // namespace bell
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_BELL_PRECONDITIONERS_JACOBINOLUPIVOT_HH
