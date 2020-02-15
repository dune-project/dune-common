exclude_from_headercheck(
  typetreetargetnodes.hh
  typetreetestswitch.hh
  typetreetestutility.hh)

dune_add_test(SOURCES testtypetree.cc
  LINK_LIBRARIES dunecommon
  COMPILE_DEFINITIONS TEST_TYPETREE)

foreach(TEST_FAILURE 2 3 5 6)
dune_add_test(NAME testtypetree_fail${TEST_FAILURE}
    SOURCES testtypetree.cc
    LINK_LIBRARIES dunecommon
    COMPILE_DEFINITIONS TEST_TYPETREE;FAILURE${TEST_FAILURE}
    EXPECT_COMPILE_FAIL)
endforeach()

dune_add_test(SOURCES testproxynode.cc
              COMPILE_DEFINITIONS TEST_TYPETREE)

dune_add_test(SOURCES testcallbacktraversal.cc
  LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES testhybridtreepath.cc)

dune_add_test(SOURCES testtreecontainer.cc
  LINK_LIBRARIES dunecommon)
