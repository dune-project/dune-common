# $Id$

if MPI
  YPROG = test-yaspgrid
  YRUN = run-yaspgrid
endif

if ALBERT
  APROG = test-albert
endif

# UGGrid needs AmiraMeshReader for now...
# !!! trick around to beat automake
if AMIRAMESH
  DO_UGTEST = test-ug
else
  DO_UGTEST =
endif

if UG
  UPROG = $(DO_UGTEST)
endif

# tests where program to build and program to run are equal
NORMALTESTS = test-sgrid $(APROG) $(UPROG)

# list of tests to run (yaspgrid is special case)
TESTS = $(NORMALTESTS) $(YRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(YPROG)

# define the programs

test_sgrid_SOURCES = test-sgrid.cc

test_yaspgrid_SOURCES = test-yaspgrid.cc
test_yaspgrid_CPPFLAGS = $(AM_CPPFLAGS) $(MPI_CPPFLAGS)
# !! last lib a bit weird...
test_yaspgrid_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS) ../../common/libcommon.la

# this implicitly checks the autoconf-test as well...
test_albert_SOURCES = test-albert.cc
test_albert_CPPFLAGS = $(AM_CPPFLAGS) $(ALBERT_CPPFLAGS)
test_albert_LDFLAGS = $(ALBERT_LDFLAGS) $(ALBERT_LIBS)

test_ug_SOURCES = test-ug.cc
test_ug_CPPFLAGS = $(AM_CPPFLAGS) $(UG_CPPFLAGS) $(AMIRAMESH_CPPFLAGS)
test_ug_LDFLAGS = $(UG_LDFLAGS) $(UG_LIBS) $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS)
