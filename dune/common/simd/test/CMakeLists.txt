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


# as of Vc-1.3.2: Vc/common/simdarray.h:561: SimdArray<T, N> may only be used
# with T = { double, float, int32_t, uint32_t, int16_t, uint16_t }
set(VCTEST_TYPES
  "std::int16_t"
  "std::uint16_t"
  "std::int32_t"
  "std::uint32_t"
  "float"
  "double")

include(DuneInstance)
# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(
  COLLECTORS HEADER BODY
  OUTPUTS VCTEST_SOURCES
)

# Vc::Vector and Vc::Mask
dune_instance_template(NAME VECTOR
  HEADER "\
extern template void UnitTest::checkVector<Vc::Vector<@SCALAR@> >();
extern template void UnitTest::checkMask<Vc::Mask<@SCALAR@> >();
"
  BODY "\
test.checkVector<Vector<@SCALAR@> >();
"
  FILES vctest_vector.cc vctest_mask.cc)

# Vc::SimdArray and Vc::SimdMaskArray
dune_instance_template(NAME ARRAY
  HEADER "\
extern template void UnitTest::checkVector<Vc::SimdArray<@SCALAR@, @LANES@> >();
extern template void UnitTest::checkMask<Vc::SimdMaskArray<@SCALAR@, @LANES@> >();
"
  BODY "\
test.checkVector<SimdArray<@SCALAR@, @LANES@> >();
"
  FILES vctest_simdarray.cc vctest_simdmaskarray.cc)

foreach(SCALAR IN LISTS VCTEST_TYPES)
  dune_instance_add(TEMPLATE VECTOR ID "${SCALAR}")
endforeach()

foreach(SCALAR IN LISTS VCTEST_TYPES)
  foreach(LANES IN ITEMS 4 8)
    dune_instance_add(TEMPLATE ARRAY ID "${SCALAR}_${LANES}")
  endforeach(LANES IN ITEMS 4 8)
endforeach()

dune_instance_end(FILES vctest.hh vctest.cc)

dune_add_test(NAME vctest
  SOURCES vctest.cc ${VCTEST_SOURCES}
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vctest)
# no need to install vctest.hh, used by vctest*.cc only

dune_add_test(NAME looptest
  SOURCES
    looptest.cc
    looptest_bool.cc

    looptest_char.cc
    looptest_unsigned_char.cc
    looptest_signed_char.cc

    looptest_short.cc
    looptest_unsigned_short.cc
    looptest_int.cc
    looptest_unsigned.cc
    looptest_long.cc
    looptest_unsigned_long.cc
    looptest_long_long.cc
    looptest_unsigned_long_long.cc

    looptest_float.cc
    looptest_double.cc
    looptest_long_double.cc
  LINK_LIBRARIES dunecommon
)
