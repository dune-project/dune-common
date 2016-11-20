# We need to explicitly instantiate the test for small groups of types -- else
# the compiler will eat excessive amounts of memory.  This way it seems to
# stay below 1GByte (with g++ 4.9.2 -O0 -g on x86_64 GNU/Linux).
dune_add_test(NAME standardtest
  SOURCES
    standardtest.cc
    standardtest_bool.cc
    standardtest_char.cc
    standardtest_complex.cc
    standardtest_floating.cc
    standardtest_signed.cc
    standardtest_unsigned.cc
    standardtest_wchar.cc
  LINK_LIBRARIES dunecommon
)
# no need to install standardtest.hh, used by standardtest*.cc only