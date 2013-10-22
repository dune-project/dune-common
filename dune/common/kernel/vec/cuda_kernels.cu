// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <cublas.h>

namespace Dune
{
  namespace Cuda
  {
    //-------------- sum ---------------
    template <typename DT_>
    __global__ void device_sum(DT_ * r, const DT_ * x, const DT_ * y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] + y[idx];
    }

    template <typename DT_>
    DT_ * sum(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_sum<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* sum(float*, const float*, const float*, unsigned long);
    template double* sum(double*, const double*, const double*, unsigned long);

    //-------------- difference ---------------
    template <typename DT_>
    __global__ void device_difference(DT_ * r, const DT_ * x, const DT_ * y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] - y[idx];
    }

    template <typename DT_>
    DT_ * difference(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_difference<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* difference(float*, const float*, const float*, unsigned long);
    template double* difference(double*, const double*, const double*, unsigned long);

    //-------------- element_product ---------------
    template <typename DT_>
    __global__ void device_element_product(DT_ * r, const DT_ * x, const DT_ * y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] * y[idx];
    }

    template <typename DT_>
    DT_ * element_product(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_element_product<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* element_product(float*, const float*, const float*, unsigned long);
    template double* element_product(double*, const double*, const double*, unsigned long);

    //-------------- element_division ---------------
    template <typename DT_>
    __global__ void device_element_division(DT_ * r, const DT_ * x, const DT_ * y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] / y[idx];
    }

    template <typename DT_>
    DT_ * element_division(DT_ * r, const DT_* x, const DT_ * y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_element_division<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* element_division(float*, const float*, const float*, unsigned long);
    template double* element_division(double*, const double*, const double*, unsigned long);

    //-------------- sum_scalar ---------------
    template <typename DT_>
    __global__ void device_sum_scalar(DT_ * r, const DT_ * x, DT_ y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] + y;
    }

    template <typename DT_>
    DT_ * sum_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_sum_scalar<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* sum_scalar(float*, const float*, float, unsigned long);
    template double* sum_scalar(double*, const double*, double, unsigned long);

    //-------------- difference_scalar ---------------
    template <typename DT_>
    __global__ void device_difference_scalar(DT_ * r, const DT_ * x, DT_ y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] - y;
    }

    template <typename DT_>
    DT_ * difference_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_difference_scalar<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* difference_scalar(float*, const float*, float, unsigned long);
    template double* difference_scalar(double*, const double*, double, unsigned long);

    //-------------- product_scalar ---------------
    template <typename DT_>
    __global__ void device_product_scalar(DT_ * r, const DT_ * x, DT_ y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] * y;
    }

    template <typename DT_>
    DT_ * product_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_product_scalar<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* product_scalar(float*, const float*, float, unsigned long);
    template double* product_scalar(double*, const double*, double, unsigned long);

    //-------------- division_scalar ---------------
    template <typename DT_>
    __global__ void device_division_scalar(DT_ * r, const DT_ * x, DT_ y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] / y;
    }

    template <typename DT_>
    DT_ * division_scalar(DT_ * r, const DT_* x, DT_ y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_division_scalar<<<grid, block>>>(r, x, y, size);
      return r;
    }

    template float* division_scalar(float*, const float*, float, unsigned long);
    template double* division_scalar(double*, const double*, double, unsigned long);

    //-------------- axpy ---------------
    template <typename DT_>
    __global__ void device_axpy(DT_ * r, const DT_ * x, DT_ a, const DT_ * y, const unsigned long count)
    {
      unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      if (idx >= count)
        return;
      r[idx] = x[idx] * a + y[idx];
    }

    template <typename DT_>
    DT_ * axpy(DT_ * r, const DT_* x, DT_ a, const DT_ * y, const unsigned long size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((size)/(double)(block.x));
      device_axpy<<<grid, block>>>(r, x, a, y, size);
      return r;
    }

    template float* axpy(float*, const float*, float, const float*, unsigned long);
    template double* axpy(double*, const double*, double, const double*, unsigned long);

    //-------------- dot ---------------
    float device_dot(const float * x, const float * y, const unsigned long size)
    {
        return cublasSdot(size, x, 1, y, 1);
    }

    double device_dot(const double * x, const double * y, const unsigned long size)
    {
        return cublasDdot(size, x, 1, y, 1);
    }

    template <typename DT_>
    DT_ dot(const DT_ * x, const DT_* y, const unsigned long size)
    {
      cublasInit();
      DT_ r = device_dot(x, y, size);
      cublasShutdown();
      return r;
    }

    template float dot(const float*, const float*, unsigned long);
    template double dot(const double*, const double*, unsigned long);

    //-------------- two_norm2 ---------------
    float device_two_norm2(const float * x, const unsigned long size)
    {
        return cublasSnrm2(size, x, 1);
    }

    double device_two_norm2(const double * x, const unsigned long size)
    {
        return cublasDnrm2(size, x, 1);
    }

    template <typename DT_>
    DT_ two_norm2(const DT_ * x, const unsigned long size)
    {
      cublasInit();
      DT_ r = device_two_norm2(x, size);
      cublasShutdown();
      return r;
    }

    template float two_norm2(const float*, unsigned long);
    template double two_norm2(const double*, unsigned long);

    //-------------- one_norm ---------------
    float device_one_norm(const float * x, const unsigned long size)
    {
        return cublasSasum(size, x, 1);
    }

    double device_one_norm(const double * x, const unsigned long size)
    {
        return cublasDasum(size, x, 1);
    }

    template <typename DT_>
    DT_ one_norm(const DT_ * x, const unsigned long size)
    {
      cublasInit();
      DT_ r = device_one_norm(x, size);
      cublasShutdown();
      return r;
    }

    template float one_norm(const float*, unsigned long);
    template double one_norm(const double*, unsigned long);

    //-------------- infinity_norm ---------------
    int device_infinity_norm(const float * x, const unsigned long size)
    {
        return cublasIsamax(size, x, 1) - 1;
    }

    int device_infinity_norm(const double * x, const unsigned long size)
    {
        return cublasIdamax(size, x, 1) - 1;
    }

    template <typename DT_>
    DT_ infinity_norm(const DT_ * x, const unsigned long size)
    {
      cublasInit();
      int i = device_infinity_norm(x, size);
      cublasShutdown();
      DT_ r;
      cudaMemcpy(&r, x + i, sizeof(DT_), cudaMemcpyDeviceToHost);
      return r;
    }

    template float infinity_norm(const float*, unsigned long);
    template double infinity_norm(const double*, unsigned long);
  }
}
