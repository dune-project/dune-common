// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ISTL_ELL_CUDA_KERNELS_HH
#define DUNE_ISTL_ELL_CUDA_KERNELS_HH

namespace Dune
{
  namespace Cuda
  {
    template <typename DT_>
    void mv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size);

    template <typename DT_>
    void umv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size);

    template <typename DT_>
    void mmv(const DT_ * y, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size);

    template <typename DT_>
    void usmv(DT_ alpha, const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size);
  }
}

#endif
