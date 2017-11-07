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


include(GeneratedInstances)
# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
begin_instances(HEADER vctest.hh BODY vctest.cc OUTPUTS VCTEST_SOURCES)

# as of Vc-1.3.2: Vc/common/simdarray.h:561: SimdArray<T, N> may only be used
# with T = { double, float, int32_t, uint32_t, int16_t, uint16_t }
set(VCTEST_TYPES
  "std::int16_t"
  "std::uint16_t"
  "std::int32_t"
  "std::uint32_t"
  "float"
  "double")

# Vc::Vector and Vc::Mask
define_template(NAME VECTOR
  BODY "test.checkVector<Vector<@SCALAR@> >();\n"
  HEADER "extern template void UnitTest::checkVector<Vc::Vector<@SCALAR@> >();\n"
  FILE "vctest_vector.cc.in")
define_template(NAME MASK
  HEADER "extern template void UnitTest::checkMask<Vc::Mask<@SCALAR@> >();\n"
  FILE "vctest_mask.cc.in")

foreach(SCALAR IN LISTS VCTEST_TYPES)
  add_instance(TEMPLATE VECTOR NAME "${SCALAR}")
  add_instance(TEMPLATE MASK   NAME "${SCALAR}")
endforeach()

# Vc::SimdArray and Vc::SimdMaskArray
define_template(NAME SIMDARRAY
  BODY "test.checkVector<SimdArray<@SCALAR@, @LANES@> >();\n"
  HEADER
    "extern template void UnitTest::checkVector<Vc::SimdArray<@SCALAR@, @LANES@> >();\n"
  FILE "vctest_simdarray.cc.in")
define_template(NAME SIMDMASKARRAY
  HEADER
    "extern template void UnitTest::checkMask<Vc::SimdMaskArray<@SCALAR@, @LANES@> >();\n"
  FILE "vctest_simdmaskarray.cc.in")

foreach(SCALAR IN LISTS VCTEST_TYPES)
  foreach(LANES IN ITEMS 4 8)
    add_instance(TEMPLATE SIMDARRAY     NAME "${SCALAR}_${LANES}")
    add_instance(TEMPLATE SIMDMASKARRAY NAME "${SCALAR}_${LANES}")
  endforeach(LANES IN ITEMS 4 8)
endforeach()

end_instances()

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
