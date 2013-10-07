// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_BLOCKDIAGONAL_EXTRACTBELLDIAGONAL_HH
#define DUNE_COMMON_KERNEL_BLOCKDIAGONAL_EXTRACTBELLDIAGONAL_HH

#include <cstdint>
#include <cassert>

#include <dune/common/memory/traits.hh>
#include <dune/common/kernel/utility.hh>
#include <dune/common/kernel/instantiation.hh>

namespace Dune {
  namespace Kernel {
    namespace block_diagonal {

      namespace blocked {

        template<typename T1,
                 typename T2,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void extract_bell_diagonal(T1* DUNE_RESTRICT diag_data,
                                   const T2* DUNE_RESTRICT data,
                                   const size_type* DUNE_RESTRICT col,
                                   const size_type* DUNE_RESTRICT kernel_block_offset,
                                   const size_type* DUNE_RESTRICT row_length,
                                   size_type n,
                                   size_type block_height,
                                   size_type block_width,
                                   size_type kernel_offset) DUNE_NOINLINE;

        template<typename T1,
                 typename T2,
                 typename size_type,
                 size_type alignment,
                 size_type kernel_block_size>
        void extract_bell_diagonal(T1* DUNE_RESTRICT diag_data,
                                   const T2* DUNE_RESTRICT data,
                                   const size_type* DUNE_RESTRICT col,
                                   const size_type* DUNE_RESTRICT kernel_block_offset,
                                   const size_type* DUNE_RESTRICT row_length,
                                   size_type n,
                                   size_type block_height,
                                   size_type block_width,
                                   size_type kernel_offset)
        {
          DUNE_ASSUME_ALIGNED(diag_data,T1,alignment);
          DUNE_ASSUME_ALIGNED(data,T2,alignment);
          DUNE_ASSUME_ALIGNED(col,size_type,alignment);
          DUNE_ASSUME_ALIGNED(kernel_block_offset,size_type,alignment/kernel_block_size);
          DUNE_ASSUME_ALIGNED(row_length,size_type,alignment);
          size_type offset = 0;
          for (size_type block = 0; block < n; ++block)
            {
              size_type cols = (kernel_block_offset[block+1] - kernel_block_offset[block]) >> Memory::block_size_log2<kernel_block_size>::value;

              // work through the SIMD blocks sequentally - there's no calculations here...
              for (int i = 0; i < kernel_block_size; ++i)
                {
                  // find diagonal matrix block
                  size_type row_start = offset*kernel_block_size + i;
                  size_type row_offset = block * kernel_block_size + i;
                  size_type row_index = kernel_offset + row_offset;
                  size_type l = 0, r = row_length[row_offset] - 1;
                  while (l < r)
                    {
                      size_type q = (l+r)/2;
                      if (row_index <= col[row_start + q * kernel_block_size])
                        r = q;
                      else
                        l = q + 1;
                    }
                  assert(col[row_start + l * kernel_block_size] == row_index);
                  size_type diag_j = l;

                  // copy block to diagonal matrix
                  for (int ii = 0; ii < block_height; ++ii)
                    for (int jj = 0; jj < block_width; ++jj)
                      diag_data[((block*block_height + ii) * block_width + jj) * kernel_block_size + i] = data[(((offset + diag_j) * block_height + ii) * block_width + jj) * kernel_block_size + i];
                }
              offset += cols;
            }
        }


#ifndef DOXYGEN

#define DECLARE_KERNEL(T1,T2,I,alignment,kernel_block_size)             \
        template                                                        \
        void extract_bell_diagonal<T1,T2,I,alignment,kernel_block_size>( \
          T1* DUNE_RESTRICT diag_data,                                  \
          const T2* DUNE_RESTRICT data,                                 \
          const I*  DUNE_RESTRICT col,                                  \
          const I*  DUNE_RESTRICT kernel_block_offset,                  \
          const I*  DUNE_RESTRICT row_length,                           \
          I n,                                                          \
          I block_height,                                               \
          I block_width,                                                \
          I kernel_offset                                               \
          );

        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::size_t),ALIGNOF_SIZE_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(double,double,std::int32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::size_t),ALIGNOF_FLOAT)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::uint32_t),ALIGNOF_UINT32_T)
        DUNE_KERNEL_INSTANTIATE_BLOCKED(DECLARE_KERNEL,DUNE_KERNEL_ARGS(float,float,std::int32_t),ALIGNOF_UINT32_T)

#undef DECLARE_KERNEL

#endif // DOXYGEN

      } // namespace blocked
    } // namespace block_diagonal
  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_BLOCKDIAGONAL_EXTRACTBELLDIAGONAL_HH
