# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libell.la

libell_la_SOURCES =				\
  mmv.cc					\
  mv.cc						\
  preconditioners/jacobi.cc			\
  umv.cc					\
  usmv.cc

ellincludedir = $(includedir)/dune/common/kernel/ell
ellinclude_HEADERS =				\
  mmv.hh					\
  mv.hh						\
  preconditioners/jacobi.hh			\
  umv.hh					\
  usmv.hh \
  cuda_kernels.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt

if CUDA

LINK = $(CC) -o $@ -lcudart -L/sfw/cuda/5.5/lib64 -lcublas
lib_LIBRARIES = libell_cuda.a
libell_cuda_a_SOURCES = cuda_kernels.cu

NVCC = nvcc
NVCCFLAGS = -g -v --ptxas-options=-v -O3 -I$(top_srcdir) $(DEFS) -use_fast_math -arch sm_21

.cu.o:
		$(NVCC) $(NVCCFLAGS)  -c -o $@ $<

endif
