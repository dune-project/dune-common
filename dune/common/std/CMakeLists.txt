install(FILES
        constexpr.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/std)

# Install some test headers, because they get used by tests in other modules
# We do this here as test will not be considered for make install
install(FILES test/iteratortest.hh DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
