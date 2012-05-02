# Create a custom target for building the tests.
# Thus they will not be built by make all any more.
# Actually I wanted this target to be a dependency
# of make test but that is currently not possible.
# See http://public.kitware.com/Bug/view.php?id=8438
# This really sucks!
# Therefore currently make build_tests has to be called
# before make test.

# enable testing before adding the subdirectories.
# Thus we can add the tests in the subdirectories
# where they actually are.
enable_testing()

#activate CTest
include(CTest)

#include our macros
include(DuneTestMacros)
