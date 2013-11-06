// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <cublas.h>

namespace Dune
{
  namespace Cuda
  {
    //-------------- mv ---------------
    template <typename DT_>
    __global__ void device_mv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      const unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      const unsigned long row = idx;
      if (row >= rows)
        return;

      DT_ r(0);
      const unsigned long chunk(row / rows_per_chunk);
      const unsigned long local_row(row % rows_per_chunk);
      const unsigned long chunk_end( (chunk == chunks - 1) ? allocated_size : cs[chunk+1]);

      for (unsigned long pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol += rows_per_chunk)
          r += x[col[pcol]] * data[pcol];
      y[row] = r;
    }

    template <typename DT_>
    void mv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_mv<<<grid, block>>>(x, y, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void mv(const float*, float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void mv(const double*, double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- umv ---------------
    template <typename DT_>
    __global__ void device_umv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      const unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      const unsigned long row = idx;
      if (row >= rows)
        return;

      DT_ r(y[row]);
      const unsigned long chunk(row / rows_per_chunk);
      const unsigned long local_row(row % rows_per_chunk);
      const unsigned long chunk_end( (chunk == chunks - 1) ? allocated_size : cs[chunk+1]);

      for (unsigned long pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol += rows_per_chunk)
          r += x[col[pcol]] * data[pcol];
      y[row] = r;
    }

    template <typename DT_>
    void umv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_umv<<<grid, block>>>(x, y, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void umv(const float*, float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void umv(const double*, double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- mmv ---------------
    template <typename DT_>
    __global__ void device_mmv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      const unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      const unsigned long row = idx;
      if (row >= rows)
        return;

      DT_ r(y[row]);
      const unsigned long chunk(row / rows_per_chunk);
      const unsigned long local_row(row % rows_per_chunk);
      const unsigned long chunk_end( (chunk == chunks - 1) ? allocated_size : cs[chunk+1]);

      for (unsigned long pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol += rows_per_chunk)
          r -= x[col[pcol]] * data[pcol];
      y[row] = r;
    }

    template <typename DT_>
    void mmv(const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_mmv<<<grid, block>>>(x, y, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void mmv(const float*, float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void mmv(const double*, double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- usmv ---------------
    template <typename DT_>
    __global__ void device_usmv(DT_ alpha, const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      const unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      const unsigned long row = idx;
      if (row >= rows)
        return;

      DT_ r(y[row]);
      const unsigned long chunk(row / rows_per_chunk);
      const unsigned long local_row(row % rows_per_chunk);
      const unsigned long chunk_end( (chunk == chunks - 1) ? allocated_size : cs[chunk+1]);

      for (unsigned long pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol += rows_per_chunk)
          r += x[col[pcol]] * data[pcol];
      y[row] = r * alpha;
    }

    template <typename DT_>
    void usmv(DT_ alpha, const DT_ * x, DT_ * y, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_usmv<<<grid, block>>>(alpha, x, y, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void usmv(float, const float*, float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void usmv(double, const double*, double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- sequential_jacobi ---------------
    template <typename DT_>
    __global__ void device_sequential_jacobi(const DT_ * v, const DT_ * d, DT_ * v_new, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      const unsigned long idx = threadIdx.x + blockDim.x * blockIdx.x;
      const unsigned long row = idx;
      if (row >= rows)
        return;

      DT_ rhs(d[row]);
      DT_ diag(DT_(1));
      const unsigned long chunk(row / rows_per_chunk);
      const unsigned long local_row(row % rows_per_chunk);
      const unsigned long chunk_end( (chunk == chunks - 1) ? allocated_size : cs[chunk+1]);

      for (unsigned long pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol += rows_per_chunk)
      {
        if (row == col[pcol])
        {
          diag = data[pcol];
        }
        else
        {
          rhs -= v[col[pcol]] * data[pcol];
        }
      }
      diag = (diag == DT_(0)) ? DT_(1) : diag;
      v_new[row] = rhs / diag;
    }

    template <typename DT_>
    void sequential_jacobi(const DT_ * v, const DT_ * d, DT_ * v_new,
        const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_sequential_jacobi<<<grid, block>>>(v, d, v_new, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void sequential_jacobi(const float*, const float*, float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void sequential_jacobi(const double*, const double*, double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
  }
}
