include(DuneCMakeCompat)

# We need to explicitly instantiate the tests for small groups of types --
# else the compiler will eat excessive amounts of memory.  This way it seems
# to stay below 1GByte (with g++ 4.9.2 -O0 -g on x86_64 GNU/Linux, looking at
# standardtest).
include(DuneInstance)

set(TYPES
  char "unsigned char" "signed char"
  short int long "long long"
  "unsigned short" unsigned "unsigned long" "unsigned long long"
  wchar_t char16_t char32_t
  bool
  float double "long double"
  std::complex<float> std::complex<double> "std::complex<long double>"
  )

# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES looptest.hh looptest.cc)
foreach(SCALAR IN LISTS TYPES)
  dune_instance_add(ID "${SCALAR}" FILES looptest_vector.cc)
endforeach()
dune_instance_end()

dune_list_filter(DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME looptest
  SOURCES ${DUNE_INSTANCE_GENERATED} looptest_mask.cc
  LINK_LIBRARIES dunecommon
)
# no need to install looptest.hh, used by looptest*.cc only



set(TYPES
  char "unsigned char" "signed char"
  short int long "long long"
  "unsigned short" unsigned "unsigned long" "unsigned long long"
  wchar_t char16_t char32_t
  bool
  float double "long double"
  std::complex<float> std::complex<double> "std::complex<long double>")

# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES standardtest.hh standardtest.cc)
foreach(SCALAR IN LISTS TYPES)
  dune_instance_add(ID "${SCALAR}" FILES standardtest_vector.cc)
endforeach()
dune_instance_end()

dune_list_filter(DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME standardtest
  SOURCES ${DUNE_INSTANCE_GENERATED} standardtest_mask.cc
  LINK_LIBRARIES dunecommon
)
# no need to install standardtest.hh, used by standardtest*.cc only


# as of Vc-1.3.2: Vc/common/simdarray.h:561: SimdArray<T, N> may only be used
# with T = { double, float, int32_t, uint32_t, int16_t, uint16_t }
set(VCTEST_TYPES std::int16_t std::uint16_t std::int32_t std::uint32_t float double)

# Generate files with instanciations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES vctest.hh vctest.cc)

# Vc::Vector and Vc::Mask
foreach(SCALAR IN LISTS VCTEST_TYPES)
  dune_instance_add(TEMPLATE VECTOR ID "${SCALAR}"
    FILES vctest_vector.cc vctest_mask.cc)
endforeach()

# Vc::SimdArray and Vc::SimdMaskArray
foreach(SCALAR IN LISTS VCTEST_TYPES)
  foreach(LANES IN ITEMS 4 8)
    dune_instance_add(TEMPLATE ARRAY ID "${SCALAR}_${LANES}"
      FILES vctest_simdarray.cc vctest_simdmaskarray.cc)
  endforeach(LANES IN ITEMS 4 8)
endforeach()

dune_instance_end()

dune_list_filter(DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])

dune_add_test(NAME vctest
  SOURCES ${DUNE_INSTANCE_GENERATED}
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vctest)
# no need to install vctest.hh, used by vctest*.cc only
