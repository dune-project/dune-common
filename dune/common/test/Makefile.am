# -*- tab-width: 4; indent-tabs-mode: nil -*-

TESTPROGS = \
    arraylisttest \
    arraytest \
    bigunsignedinttest \
    bitsetvectortest \
    calloncetest \
    check_fvector_size \
    conversiontest \
    diagonalmatrixtest \
    dynmatrixtest \
    dynvectortest \
    eigenvaluestest \
    enumsettest \
    fmatrixtest \
    fvectortest \
    gcdlcmtest \
    integersequence \
    iteratorfacadetest \
    iteratorfacadetest2 \
    lrutest \
    mpicollectivecommunication \
    mpiguardtest \
    mpihelpertest \
    mpihelpertest2 \
    nullptr-test \
    pathtest \
    parametertreelocaletest \
    parametertreetest \
    poolallocatortest \
    shared_ptrtest_config \
    singletontest \
    streamtest \
    stringutilitytest \
    testdebugallocator \
    testdebugallocator_fail1 \
    testdebugallocator_fail2 \
    testdebugallocator_fail3 \
    testdebugallocator_fail4 \
    testdebugallocator_fail5 \
    testfloatcmp \
    tuplestest \
    tupleutilitytest \
    typetraitstest \
    utilitytest

# which tests to run
COMPILE_XFAIL=$(DUNE_COMMON_BIN)/xfail-compile-tests

COMPILE_XFAIL_TESTS = \
    check_fvector_size_fail1 \
    check_fvector_size_fail2 \
    genericiterator_compile_fail \
    nullptr-test-fail

compile_XFAIL:
	for i in $(COMPILE_XFAIL_TESTS); do \
	  if $(MAKE) $$i; then \
	    echo "ERROR: target $$i did compile, although it is listed in the COMPILE_XFAIL_TESTS"; \
	    exit 1; \
	  else true; \
	  fi; \
	done

EXTRA_PROGRAMS = $(COMPILE_XFAIL_TESTS) sllisttest

TESTS = $(TESTPROGS) $(COMPILE_XFAIL)

XFAIL_TESTS = \
    testdebugallocator_fail1 \
    testdebugallocator_fail2 \
    testdebugallocator_fail3 \
    testdebugallocator_fail4 \
    testdebugallocator_fail5

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

noinst_HEADERS = dummyiterator.hh iteratorfacadetest.hh

# Install some test headers, because they get used by tests in other modules
testincludedir = $(includedir)/dune/common/test
testinclude_HEADERS = checkmatrixinterface.hh iteratortest.hh

# define the programs (in alphabetical order)
arraylisttest_SOURCES = arraylisttest.cc

arraytest_SOURCES = arraytest.cc

bigunsignedinttest_SOURCES=bigunsignedinttest.cc
bigunsignedinttest_CPPFLAGS = $(AM_CPPFLAGS)

bitsetvectortest_SOURCES = bitsetvectortest.cc

calloncetest_SOURCES = calloncetest.cc

check_fvector_size_fail1_SOURCES = check_fvector_size_fail.cc
check_fvector_size_fail1_CPPFLAGS = $(AM_CPPFLAGS) -DDIM=1

check_fvector_size_fail2_SOURCES = check_fvector_size_fail.cc
check_fvector_size_fail2_CPPFLAGS = $(AM_CPPFLAGS) -DDIM=3

check_fvector_size_SOURCES = check_fvector_size.cc

conversiontest_SOURCES = conversiontest.cc

diagonalmatrixtest_SOURCES = diagonalmatrixtest.cc

dynmatrixtest_SOURCES = dynmatrixtest.cc

dynvectortest_SOURCES = dynvectortest.cc

eigenvaluestest_SOURCES = eigenvaluestest.cc
eigenvaluestest_LDADD = $(LAPACK_LIBS) $(LDADD) $(BLAS_LIBS) $(LIBS) $(FLIBS)

enumsettest_SOURCES = enumsettest.cc

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_LDADD = $(LAPACK_LIBS) $(LDADD) $(BLAS_LIBS) $(LIBS) $(FLIBS)

fvectortest_SOURCES = fvectortest.cc

gcdlcmtest_SOURCES = gcdlcmtest.cc

genericiterator_compile_fail_SOURCES = genericiterator_compile_fail.cc

integersequence_SOURCES = integersequence.cc
integersequence_CPPFLAGS = $(AM_CPPFLAGS)

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

iteratorfacadetest2_SOURCES = iteratorfacadetest2.cc

lrutest_SOURCES = lrutest.cc

mpicollectivecommunication_SOURCES = mpicollectivecommunication.cc
mpicollectivecommunication_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpicollectivecommunication_LDADD = $(DUNEMPILIBS) $(LDADD)
mpicollectivecommunication_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpiguardtest_SOURCES = mpiguardtest.cc
mpiguardtest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpiguardtest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpiguardtest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpihelpertest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpihelpertest2_SOURCES = mpihelpertest.cc
mpihelpertest2_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) -DMPIHELPER_PREINITIALIZE
mpihelpertest2_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest2_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

nullptr_test_SOURCES = nullptr-test.cc nullptr-test2.cc
nullptr_test_fail_SOURCES = nullptr-test.cc
nullptr_test_fail_CPPFLAGS = $(AM_CPPFLAGS) -DFAIL

parametertreelocaletest_SOURCES = parametertreelocaletest.cc
parametertreetest_SOURCES = parametertreetest.cc

pathtest_SOURCES = pathtest.cc

poolallocatortest_SOURCES = poolallocatortest.cc

shared_ptrtest_config_SOURCES = shared_ptrtest.cc

singletontest_SOURCES = singletontest.cc

sllisttest_SOURCES = sllisttest.cc

sourcescheck_NOSOURCES = timing.cc

streamtest_SOURCES = streamtest.cc

stringutilitytest_SOURCES = stringutilitytest.cc

testdebugallocator_SOURCES = testdebugallocator.cc
testdebugallocator_CPPFLAGS = $(AM_CPPFLAGS)

testdebugallocator_fail1_SOURCES = testdebugallocator.cc
testdebugallocator_fail1_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE1

testdebugallocator_fail2_SOURCES = testdebugallocator.cc
testdebugallocator_fail2_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE2

testdebugallocator_fail3_SOURCES = testdebugallocator.cc
testdebugallocator_fail3_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE3

testdebugallocator_fail4_SOURCES = testdebugallocator.cc
testdebugallocator_fail4_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE4 -DDEBUG_ALLOCATOR_KEEP=1

testdebugallocator_fail5_SOURCES = testdebugallocator.cc
testdebugallocator_fail5_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE5

testfloatcmp_SOURCES = testfloatcmp.cc

tuplestest_SOURCES = tuplestest.cc

tupleutilitytest_SOURCES = tupleutilitytest.cc

typetraitstest_SOURCES = typetraitstest.cc

utilitytest_SOURCES = utilitytest.cc



include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt dummy.f
