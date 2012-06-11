# $Id$

SUBDIRS = . test parallel

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES =				\
	fmatrixev.cc \
	ios_state.cc				\
	parametertree.cc                        \
	parametertreeparser.cc			\
	path.cc					\
	exceptions.cc				\
	stdstreams.cc
libcommon_la_LIBADD = $(LAPACK_LIBS)

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
	debugstream.hh				\
	deprecated.hh				\
	densematrix.hh				\
	densevector.hh				\
	documentation.hh			\
	dynmatrix.hh				\
	dynvector.hh				\
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
	indent.hh				\
	interfaces.hh				\
	ios_state.hh				\
	iteratorfacades.hh			\
	lcm.hh					\
	lru.hh					\
	math.hh					\
	matvectraits.hh \
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
	precision.hh				\
	propertymap.hh				\
	reservedvector.hh			\
	shared_ptr.hh				\
	singleton.hh				\
	sllist.hh				\
	static_assert.hh			\
	stdstreams.hh				\
	timer.hh				\
	tuples.hh				\
	tupleutility.hh                         \
	typetraits.hh				\
	unused.hh				\
	version.hh

include $(top_srcdir)/am/global-rules
