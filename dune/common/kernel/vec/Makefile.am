# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libvec.la

libvec_la_SOURCES =				\
  add.cc					\
  assign_scalar.cc				\
  axpy.cc					\
  dot.cc					\
  infinity_norm.cc				\
  mul.cc					\
  one_norm.cc					\
  scale.cc					\
  shift.cc					\
  subtract.cc					\
  two_norm2.cc

vecincludedir = $(includedir)/dune/common/kernel/vec
vecinclude_HEADERS =				\
  add.hh					\
  assign_scalar.hh				\
  axpy.hh					\
  cuda_kernels.hh \
  dot.hh					\
  infinity_norm.hh				\
  maskeddot.hh					\
  mul.hh					\
  one_norm.hh					\
  scale.hh					\
  shift.hh					\
  subtract.hh					\
  two_norm2.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt

if CUDA

LINK = $(CC) -o $@ -lcudart -L/sfw/cuda/5.5/lib64 -lcublas
lib_LIBRARIES = libvec_cuda.a
libvec_cuda_a_SOURCES = cuda_kernels.cu

NVCC = nvcc
NVCCFLAGS = $(DUNE_CPPFLAGS)-g -v --ptxas-options=-v -O3 -I$(top_srcdir) $(DEFS) -use_fast_math -arch sm_21

.cu.o:
		$(NVCC) $(NVCCFLAGS)  -c -o $@ $<

endif
