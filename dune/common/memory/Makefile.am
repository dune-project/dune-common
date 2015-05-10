memoryincludedir = $(includedir)/dune/common/memory
memoryinclude_HEADERS =				\
  alignment.hh					\
  blocked_allocator.hh				\
  cuda_allocator.hh \
  domain.hh					\
  traits.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt

if CUDA

LINK = $CC -o $@ -lcudart -L/sfw/cuda/5.5/lib64 -lcublas
lib_LIBRARIES = libcuda_memory.a
libcuda_memory_a_SOURCES = cuda_allocator.cu

NVCC = nvcc
NVCCFLAGS = -g -v --ptxas-options=-v -O3 -I$(top_srcdir) $(DEFS) -use_fast_math -arch sm_13

.cu.o:
		$(NVCC) $(NVCCFLAGS)  -c -o $@ $<

endif
