// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <cublas.h>

namespace Dune
{
  namespace Cuda
  {
    //-------------- mv ---------------
    template <typename DT_>
    __global__ void device_mv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
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
    void mv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_mv<<<grid, block>>>(y, x, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void mv(float*, const float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void mv(double*, const double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- umv ---------------
    template <typename DT_>
    __global__ void device_umv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
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
    void umv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_umv<<<grid, block>>>(y, x, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void umv(float*, const float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void umv(double*, const double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- mmv ---------------
    template <typename DT_>
    __global__ void device_mmv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
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
    void mmv(DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_mmv<<<grid, block>>>(y, x, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void mmv(float*, const float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void mmv(double*, const double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);

    //-------------- usmv ---------------
    template <typename DT_>
    __global__ void device_usmv(DT_ alpha, DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
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
    void usmv(DT_ alpha, DT_ * y, const DT_ * x, const DT_ * data, const unsigned long * cs, const unsigned long * col,
        const unsigned long rows, const unsigned long rows_per_chunk,
        const unsigned long chunks, const unsigned long allocated_size)
    {
      unsigned long blocksize(128);
      dim3 grid;
      dim3 block;
      block.x = blocksize;
      grid.x = (unsigned)ceil((rows)/(double)(block.x));
      device_usmv<<<grid, block>>>(alpha, y, x, data, cs, col, rows, rows_per_chunk, chunks, allocated_size);
    }

    template void usmv(float, float*, const float*, const float*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
    template void usmv(double, double*, const double*, const double*, const unsigned long*, const unsigned long*,
      const unsigned long, const unsigned long,
      const unsigned long, const unsigned long);
  }
}
