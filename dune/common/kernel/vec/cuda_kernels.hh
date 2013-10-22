// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ISTL_VECTOR_CUDA_KERNELS_HH
#define DUNE_ISTL_VECTOR_CUDA_KERNELS_HH

namespace Dune
{
  namespace Cuda
  {
    template <typename DT_>
    DT_ * sum(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ * difference(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ * element_product(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ * element_division(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ * sum_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size);

    template <typename DT_>
    DT_ * difference_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size);

    template <typename DT_>
    DT_ * product_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size);

    template <typename DT_>
    DT_ * division_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size);

    template <typename DT_>
    DT_ * axpy(DT_ * r, const DT_* x, DT_ a, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ dot(const DT_ * x, const DT_ * y, const unsigned long size);

    template <typename DT_>
    DT_ two_norm2(const DT_ * x, const unsigned long size);

    template <typename DT_>
    DT_ one_norm(const DT_ * x, const unsigned long size);

    template <typename DT_>
    DT_ infinity_norm(const DT_ * x, const unsigned long size);
  }
}

#endif
