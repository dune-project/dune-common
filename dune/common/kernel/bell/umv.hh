// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_BELL_UMV_HH
#define DUNE_COMMON_KERNEL_BELL_UMV_HH

#include <cstdint>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace bell {

      namespace blocked {

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type block_size>
        void umv(T1* DUNE_RESTRICT y,
                 const T2* DUNE_RESTRICT x,
                 const T3* DUNE_RESTRICT mat_data,
                 const size_type* DUNE_RESTRICT mat_col,
                 const size_type* DUNE_RESTRICT mat_block_offset,
                 size_type n,
                 size_type block_width,
                 size_type block_height) DUNE_NOINLINE;

        template<typename T1,
                 typename T2,
                 typename T3,
                 typename size_type,
                 size_type alignment,
                 size_type block_size>
        void umv(T1* DUNE_RESTRICT y,
                 const T2* DUNE_RESTRICT x,
                 const T3* DUNE_RESTRICT mat_data,
                 const size_type* DUNE_RESTRICT mat_col,
                 const size_type* DUNE_RESTRICT mat_block_offset,
                 size_type n,
                 size_type block_width,
                 size_type block_height)
        {
          DUNE_ASSUME_ALIGNED(y,T1,alignment);
          DUNE_ASSUME_ALIGNED(x,T2,alignment);
          DUNE_ASSUME_ALIGNED(mat_data,T3,alignment);
          DUNE_ASSUME_ALIGNED(mat_col,size_type,alignment);
          DUNE_ASSUME_ALIGNED(mat_block_offset,size_type,alignment/block_size);
          size_type offset = 0;
          for (size_type block = 0; block < n; ++block)
            {
              size_type cols = (mat_block_offset[block+1] - mat_block_offset[block]) >> Memory::block_size_log2<block_size>::value;
              for (size_type j = 0; j < cols; ++j)
                for (size_type ii = 0; ii < block_height; ++ii)
                  {
                    T1 y_[block_size];
                    for (size_type i = 0; i < block_size; ++i)
                      y_[i] = y[block*block_size*block_height + ii + i*block_height];
                    for (size_type jj = 0; jj < block_width; ++jj)
                      for (size_type i = 0; i < block_size; ++i)
                        y_[i] += mat_data[(((offset + j)*block_height + ii) * block_width + jj) * block_size + i] * x[mat_col[offset*block_size + block_size*j+i] * block_width + jj];
                    for (size_type i = 0; i < block_size; ++i)
                      y[block*block_size*block_height + ii + i*block_height] = y_[i];
                  }
              offset += cols;
            }
        }


#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,T3,I,alignment,block_size)                 \
        template                                                        \
        void umv<T1,T2,T3,I,alignment,block_size>(                      \
          T1* DUNE_RESTRICT y,                                          \
          const T2* DUNE_RESTRICT x,                                    \
          const T3* DUNE_RESTRICT mat_data,                             \
          const I*  DUNE_RESTRICT mat_col,                              \
          const I*  DUNE_RESTRICT mat_block_offset,                     \
          I n,                                                          \
          I block_width,                                                \
          I block_height                                                \
          );

        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,double,std::size_t),ALIGNOF_SIZE_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,double,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,float,std::size_t),ALIGNOF_FLOAT)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,float,std::uint32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      } // namespace blocked
    } // namespace bell
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_ELL_UMV_HH
