# We need to explicitly instantiate the test for small groups of types -- else
# the compiler will eat excessive amounts of memory.  This way it seems to
# stay below 1GByte (with g++ 4.9.2 -O0 -g on x86_64 GNU/Linux).
dune_add_test(NAME standardtest
  SOURCES
    standardtest.cc

    standardtest_char.cc
    standardtest_unsigned_char.cc
    standardtest_signed_char.cc

    standardtest_short.cc
    standardtest_unsigned_short.cc
    standardtest_int.cc
    standardtest_unsigned.cc
    standardtest_long.cc
    standardtest_unsigned_long.cc
    standardtest_long_long.cc
    standardtest_unsigned_long_long.cc

    standardtest_wchar_t.cc
    standardtest_char16_t.cc
    standardtest_char32_t.cc

    standardtest_bool.cc

    standardtest_float.cc
    standardtest_double.cc
    standardtest_long_double.cc

    standardtest_complex_float.cc
    standardtest_complex_double.cc
    standardtest_complex_long_double.cc
  LINK_LIBRARIES dunecommon
)
# no need to install standardtest.hh, used by standardtest*.cc only

dune_add_test(NAME vctest
  SOURCES
    vctest.cc

    # Vc::Vector types
    vctest_vector_short.cc
    vctest_vector_unsigned_short.cc
    vctest_vector_int.cc
    vctest_vector_unsigned.cc

    vctest_vector_float.cc
    vctest_vector_double.cc

    # Vc::Mask types
    vctest_mask_short.cc
    vctest_mask_unsigned_short.cc
    vctest_mask_int.cc
    vctest_mask_unsigned.cc

    vctest_mask_float.cc
    vctest_mask_double.cc

    # Vc::SimdArray types with 4 lanes
    vctest_simdarray_int_4.cc

    # Vc::SimdMaskArray types with 4 lanes
    vctest_simdmaskarray_int_4.cc

    # Vc::SimdArray types with 8 lanes
    vctest_simdarray_int_8.cc

    # Vc::SimdMaskArray types with 8 lanes
    vctest_simdmaskarray_int_8.cc
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vctest)
# no need to install vctest.hh, used by vctest*.cc only




dune_add_test(NAME fakevectortest
  SOURCES
    fakevectortest.cc

  LINK_LIBRARIES dunecommon
)
