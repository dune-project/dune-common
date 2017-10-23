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


# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
set(VCTEST_SOURCES)
set(VCTEST_HH_CONTENT)
set(VCTEST_CC_CONTENT)

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
foreach(SCALAR IN LISTS VCTEST_TYPES)
  string(REGEX REPLACE "[^a-zA-Z0-9]+" _ MANGLED_SCALAR "${SCALAR}")

  configure_file(vctest_vector.cc.in
    "vctest_vector_${MANGLED_SCALAR}.cc")
  configure_file(vctest_mask.cc.in
    "vctest_mask_${MANGLED_SCALAR}.cc")
  set_property(
    SOURCE
      "vctest_vector_${MANGLED_SCALAR}.cc"
      "vctest_mask_${MANGLED_SCALAR}.cc"
    PROPERTY GENERATED TRUE)
  list(APPEND VCTEST_SOURCES
    "vctest_vector_${MANGLED_SCALAR}.cc"
    "vctest_mask_${MANGLED_SCALAR}.cc")
  set(VCTEST_HH_CONTENT "${VCTEST_HH_CONTENT}\
extern template void UnitTest::checkVector<Vc::Vector<${SCALAR}> >();
extern template void UnitTest::checkVector<Vc::Mask<${SCALAR}> >();
")
  set(VCTEST_CC_CONTENT "${VCTEST_CC_CONTENT}\
test.checkVector<Vector<${SCALAR}> >();
")
endforeach()

# Vc::SimdArray and Vc::SimdMaskArray
foreach(SCALAR IN LISTS VCTEST_TYPES)
  string(REGEX REPLACE "[^a-zA-Z0-9]+" _ MANGLED_SCALAR "${SCALAR}")
  foreach(LANES IN ITEMS 4 8)
    configure_file(vctest_simdarray.cc.in
      "vctest_simdarray_${MANGLED_SCALAR}_${LANES}.cc")
    configure_file(vctest_simdmaskarray.cc.in
      "vctest_simdmaskarray_${MANGLED_SCALAR}_${LANES}.cc")
    set_property(
      SOURCE
        "vctest_simdarray_${MANGLED_SCALAR}_${LANES}.cc"
        "vctest_simdmaskarray_${MANGLED_SCALAR}_${LANES}.cc"
      PROPERTY GENERATED TRUE)
    list(APPEND VCTEST_SOURCES
      "vctest_simdarray_${MANGLED_SCALAR}_${LANES}.cc"
      "vctest_simdmaskarray_${MANGLED_SCALAR}_${LANES}.cc")
    set(VCTEST_HH_CONTENT "${VCTEST_HH_CONTENT}\
extern template void UnitTest::checkVector<Vc::SimdArray<${SCALAR}, ${LANES}> >();
extern template void UnitTest::checkVector<Vc::SimdMaskArray<${SCALAR}, ${LANES}> >();
")
    set(VCTEST_CC_CONTENT "${VCTEST_CC_CONTENT}\
test.checkVector<SimdArray<${SCALAR}, ${LANES}> >();
")
  endforeach(LANES IN ITEMS 4 8)
endforeach()

configure_file(vctest.hh.in vctest.hh)
configure_file(vctest.cc.in vctest.cc)
set_property(SOURCE vctest.cc vctest.hh PROPERTY GENERATED TRUE)

dune_add_test(NAME vctest
  SOURCES vctest.cc ${VCTEST_SOURCES}
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vctest)
# no need to install vctest.hh, used by vctest*.cc only
