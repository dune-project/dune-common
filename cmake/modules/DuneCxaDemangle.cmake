# This module checks whether the compiler supports the
# abi::__cxa_demangle function required to
# make the type names returned by typeid() human-readable
#
# It sets the variable :code:`HAVE_CXA_DEMANGLE` with the result.
#

include(CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("#include <typeinfo>
#include <cxxabi.h>
int main(void){
    int foobar = 0;
    const char *foo = typeid(foobar).name();
    int status;
    char *demangled = abi::__cxa_demangle( foo, 0, 0, &status );
}" HAVE_CXA_DEMANGLE)
