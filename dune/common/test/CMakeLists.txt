add_executable("arraylisttest" arraylisttest.cc)
add_executable("arraytest" arraytest.cc)

add_executable("bigunsignedinttest" bigunsignedinttest.cc)
target_link_libraries("bigunsignedinttest" "dunecommon")

add_executable("bitsetvectortest" bitsetvectortest.cc)
add_executable("conversiontest" conversiontest.cc)
add_executable("deprtuplestest" deprtuplestest.cc)

add_executable("dynmatrixtest" dynmatrixtest.cc)
target_link_libraries("dynmatrixtest" "dunecommon")

add_executable("dynvectortest" dynvectortest.cc)
add_executable("enumsettest" enumsettest.cc)

add_executable("fassigntest" fassigntest.cc)
target_link_libraries("fassigntest" "dunecommon")

add_executable("fmatrixtest" fmatrixtest.cc)
target_link_libraries("fmatrixtest" "dunecommon")

add_executable("fvectortest" fvectortest.cc)
add_executable("gcdlcmtest" gcdlcmtest.cc)
add_executable("iteratorfacadetest2" iteratorfacadetest2.cc)
add_executable("iteratorfacadetest" iteratorfacadetest.cc)
add_executable("lrutest" lrutest.cc)

add_executable("mpiguardtest" mpiguardtest.cc)
target_link_libraries("mpiguardtest" "dunecommon")

add_executable("mpihelpertest" mpihelpertest.cc)
add_executable("nullptr-test" nullptr-test.cc)

#add_executable("nullptr-test2" nullptr-test2.cc)
#target_link_libraries("nullptr-test2" "dunecommon")

add_executable("parametertreetest" parametertreetest.cc)
target_link_libraries("parametertreetest" "dunecommon")

add_executable("pathtest" pathtest.cc)
target_link_libraries("pathtest" "dunecommon")

add_executable("poolallocatortest" poolallocatortest.cc)
add_executable("shared_ptrtest" shared_ptrtest.cc)
add_executable("singletontest" singletontest.cc)
add_executable("sllisttest" sllisttest.cc)
add_executable("static_assert_test" static_assert_test.cc)
#add_executable("static_assert_test_fail" static_assert_test_fail.cc)

add_executable("streamtest" streamtest.cc)
target_link_libraries("streamtest" "dunecommon")

#add_executable("testfassign" testfassign.cc)
#add_executable("testfassign2" testfassign2.cc)
#add_executable("testfconstruct" testfconstruct.cc)

add_executable("test-stack" test-stack.cc)
target_link_libraries("test-stack" "dunecommon")

add_executable("tuplestest" tuplestest.cc)
add_executable("tupleutilitytest" tupleutilitytest.cc)
add_executable("utilitytest" utilitytest.cc)
