# $Id$

SUBDIRS = . test parallel std

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES =				\
	debugallocator.cc			\
	fmatrixev.cc                            \
	dynmatrixev.cc                          \
	ios_state.cc				\
	parametertree.cc                        \
	parametertreeparser.cc			\
	path.cc					\
	exceptions.cc				\
	stdstreams.cc
libcommon_la_LIBADD = $(LAPACK_LIBS) $(BLAS_LIBS) $(LIBS) $(FLIBS)

commonincludedir = $(includedir)/dune/common
commoninclude_HEADERS = 			\
	alignment.hh				\
	array.hh				\
	arraylist.hh				\
	bartonnackmanifcheck.hh			\
	bigunsignedint.hh			\
	binaryfunctions.hh			\
	bitsetvector.hh				\
	classname.hh				\
	collectivecommunication.hh		\
	debugallocator.hh			\
	debugstream.hh				\
	deprecated.hh				\
	densematrix.hh				\
	densevector.hh				\
	diagonalmatrix.hh                       \
	documentation.hh			\
	dotproduct.hh				\
	dynmatrix.hh				\
	dynvector.hh				\
        dynmatrixev.hh                          \
	enumset.hh				\
	exceptions.hh				\
	fassign.hh				\
	float_cmp.cc				\
	float_cmp.hh				\
	fmatrix.hh				\
	fmatrixev.hh				\
	forloop.hh				\
	ftraits.hh				\
	function.hh				\
	fvector.hh				\
	gcd.hh					\
	genericiterator.hh			\
	gmpfield.hh				\
	hash.hh					\
	identitymatrix.hh                       \
	indent.hh				\
	interfaces.hh				\
	ios_state.hh				\
	iteratorfacades.hh			\
	lcm.hh					\
	lru.hh					\
	mallocallocator.hh			\
	math.hh					\
	matvectraits.hh                         \
	misc.hh					\
	mpicollectivecommunication.hh		\
	mpiguard.hh				\
	mpihelper.hh				\
	mpitraits.hh				\
	nullptr.hh				\
	parametertree.hh                        \
	parametertreeparser.hh			\
	path.hh					\
	poolallocator.hh			\
	power.hh				\
	precision.hh				\
	promotiontraits.hh			\
	propertymap.hh				\
	reservedvector.hh			\
	shared_ptr.hh				\
	singleton.hh				\
	sllist.hh				\
	static_assert.hh			\
	stdstreams.hh				\
	stringutility.hh			\
	timer.hh				\
	tuples.hh				\
	tupleutility.hh                         \
	typetraits.hh				\
	unused.hh				\
	version.hh				\
        visibility.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
