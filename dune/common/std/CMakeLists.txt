set(HEADERS
  apply.hh
  functional.hh
  make_array.hh
  type_traits.hh
)

set(DEPRECATED_HEADERS
  optional.hh
  utility.hh
  variant.hh
)

install(
  FILES ${HEADERS} ${DEPRECATED_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/std
)
