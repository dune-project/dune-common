#include <cuda.h>
#include <dune/common/memory/cuda_allocator.hh>

using namespace Dune;
using namespace Dune::Memory;
using namespace Dune::Cuda;

template <typename DT_>
typename std::allocator<DT_>::pointer CudaAllocator<DT_>::allocate(size_t n, typename std::allocator<void>::const_pointer /*hint*/)
{
  void * r;
  cudaError_t status = cudaMalloc(&r, n * sizeof(DT_));
  if (status != cudaSuccess)
    throw new std::bad_alloc;

  return (DT_*)r;
}

template <typename DT_>
void CudaAllocator<DT_>::deallocate(typename std::allocator<DT_>::pointer p, size_t /*n*/)
{
  cudaFree((void*) p);
}

template typename std::allocator<float>::pointer CudaAllocator<float>::allocate(size_t n, typename std::allocator<void>::const_pointer);
template typename std::allocator<double>::pointer CudaAllocator<double>::allocate(size_t n, typename std::allocator<void>::const_pointer);
template void CudaAllocator<float>::deallocate(typename std::allocator<float>::pointer, size_t);
template void CudaAllocator<double>::deallocate(typename std::allocator<double>::pointer, size_t);

template <typename DT_>
void Dune::Cuda::upload(DT_ * dst, const DT_ * src, size_t count)
{
  cudaMemcpy(dst, src, count * sizeof(DT_), cudaMemcpyHostToDevice);
}

template <typename DT_>
void Dune::Cuda::download(DT_ * dst, const DT_ * src, size_t count)
{
  cudaMemcpy(dst, src, count * sizeof(DT_), cudaMemcpyDeviceToHost);
}

template <typename DT_>
void Dune::Cuda::copy(DT_ * dst, const DT_ * src, size_t count)
{
  cudaMemcpy(dst, src, count * sizeof(DT_), cudaMemcpyDeviceToDevice);
}

template <typename DT_>
void Dune::Cuda::set(DT_ * dst, const DT_ & val)
{
  cudaMemcpy(dst, &val, sizeof(DT_), cudaMemcpyHostToDevice);
}

template <typename DT_>
DT_ Dune::Cuda::get(DT_ * src)
{
  DT_ result;
  cudaMemcpy(&result, src, sizeof(DT_), cudaMemcpyDeviceToHost);
  return result;
}

template void Dune::Cuda::upload(float *, const float *, size_t);
template void Dune::Cuda::upload(double *, const double *, size_t);
template void Dune::Cuda::download(float *, const float *, size_t);
template void Dune::Cuda::download(double *, const double *, size_t);
template void Dune::Cuda::copy(float *, const float *, size_t);
template void Dune::Cuda::copy(double *, const double *, size_t);
template void Dune::Cuda::set(float *, const float &);
template void Dune::Cuda::set(double *, const double &);
template float Dune::Cuda::get(float *);
template double Dune::Cuda::get(double *);
