# $Id$

SUBDIRS = . test exprtmpl

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES =				\
	configparser.cc				\
	ios_state.cc				\
	parametertree.cc                        \
	stdstreams.cc

commonincludedir = $(includedir)/dune/common
commoninclude_HEADERS = 			\
	alignment.hh				\
	array.hh				\
	arraylist.hh				\
	bartonnackmanifcheck.hh			\
	bigunsignedint.hh			\
	binaryfunctions.hh			\
	bitsetvector.hh				\
	collectivecommunication.hh		\
	configparser.hh				\
	debugstream.hh				\
	deprecated.hh				\
	enumset.hh				\
	exceptions.hh				\
	fassign.hh				\
	finitestack.hh				\
	fixedarray.hh				\
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
	geometrytype.hh				\
	gmpfield.hh				\
	interfaces.hh				\
	ios_state.hh				\
	iteratorfacades.hh			\
	lcm.hh					\
	lru.hh					\
	misc.hh					\
	mpicollectivecommunication.hh		\
	mpiguard.hh				\
	mpihelper.hh				\
	mpitraits.hh				\
	nullptr.hh				\
	parametertree.hh                        \
	polyallocator.hh			\
	poolallocator.hh			\
	precision.hh				\
	propertymap.hh				\
	shared_ptr.hh				\
	singleton.hh				\
	sllist.hh				\
	smallobject.hh				\
	static_assert.hh			\
	stdstreams.hh				\
	timer.hh				\
	tuples.hh				\
	typetraits.hh				\
	utility.hh				\
	version.hh

if EXPRESSIONTEMPLATES
commoninclude_HEADERS += exprtmpl.hh exprtmpl/scalar.inc exprtmpl/exprexpr.inc
libcommon_la_SOURCES += exprtmpl.cc
else
sourcescheck_NOSOURCES = exprtmpl.hh exprtmpl.cc
endif

include $(top_srcdir)/am/global-rules
