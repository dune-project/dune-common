# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# We need to explicitly instantiate the tests for small groups of types --
# else the compiler will eat excessive amounts of memory.  This way it seems
# to stay below 1GByte (with g++ 4.9.2 -O0 -g on x86_64 GNU/Linux, looking at
# standardtest).
include(DuneInstance)

set(TYPES
  char "unsigned char" "signed char"
  short int long "long long"
  "unsigned short" unsigned "unsigned long" "unsigned long long"
  bool
  float double "long double"
  std::complex<float> std::complex<double> "std::complex<long double>"
  )

# Generate files with instantiations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES looptest.hh looptest.cc)
foreach(SCALAR IN LISTS TYPES)
  dune_instance_add(ID "${SCALAR}")
  foreach(POINT IN ITEMS
      Type
      BinaryOpsScalarVector BinaryOpsVectorScalar)
    dune_instance_add(TEMPLATE POINT ID "${POINT}_${SCALAR}"
      FILES looptest_vector.cc)
  endforeach()
endforeach()
dune_instance_end()

list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME looptest
  SOURCES ${DUNE_INSTANCE_GENERATED}
  LINK_LIBRARIES dunecommon
)
# no need to install looptest.hh, used by looptest*.cc only



set(TYPES
  char "unsigned char" "signed char"
  short int long "long long"
  "unsigned short" unsigned "unsigned long" "unsigned long long"
  bool
  float double "long double"
  std::complex<float> std::complex<double> "std::complex<long double>")

# Generate files with instantiations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES standardtest.hh standardtest.cc)
foreach(SCALAR IN LISTS TYPES)
  dune_instance_add(ID "${SCALAR}" FILES standardtest_vector.cc)
endforeach()
dune_instance_end()

list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME standardtest
  SOURCES ${DUNE_INSTANCE_GENERATED}
  LINK_LIBRARIES dunecommon
)
# no need to install standardtest.hh, used by standardtest*.cc only


# as of Vc-1.3.2: Vc/common/simdarray.h:561: SimdArray<T, N> may only be used
# with T = { double, float, int32_t, uint32_t, int16_t, uint16_t }
set(VCTEST_TYPES std::int16_t std::uint16_t std::int32_t std::uint32_t float double)

# Generate files with instantiations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES vcarraytest.hh vcarraytest.cc)
foreach(SCALAR IN LISTS VCTEST_TYPES)
  dune_instance_add(ID "${SCALAR}")
  foreach(POINT IN ITEMS
      Type
      BinaryOpsScalarVector BinaryOpsVectorScalar
      BinaryOpsProxyVector BinaryOpsVectorProxy)
    dune_instance_add(TEMPLATE POINT ID "${POINT}_${SCALAR}"
      FILES vctest_simdarray.cc vctest_simdmaskarray.cc)
  endforeach()
endforeach()
dune_instance_end()
list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME vcarraytest
  SOURCES ${DUNE_INSTANCE_GENERATED}
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vcarraytest)
# no need to install vcarraytest.hh, used by vctest*.cc only

# Generate files with instantiations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES vcvectortest.hh vcvectortest.cc)
foreach(SCALAR IN LISTS VCTEST_TYPES)
  dune_instance_add(ID "${SCALAR}")
  foreach(POINT IN ITEMS
      Type
      BinaryOpsScalarVector BinaryOpsVectorScalar
      BinaryOpsProxyVector BinaryOpsVectorProxy)
    dune_instance_add(TEMPLATE POINT ID "${POINT}_${SCALAR}"
      FILES vctest_vector.cc vctest_mask.cc)
  endforeach()
endforeach()
dune_instance_end()
list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME vcvectortest
  SOURCES ${DUNE_INSTANCE_GENERATED}
  LINK_LIBRARIES dunecommon
  CMAKE_GUARD Vc_FOUND
)
add_dune_vc_flags(vcvectortest)
# no need to install vcvectortest.hh, used by vctest*.cc only
