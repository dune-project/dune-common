add_executable("indexsettest" indexsettest.cc)
target_link_libraries("indexsettest" "dunecommon")

add_executable("indicestest" indicestest.cc)
target_link_libraries("indicestest" "dunecommon")

add_executable("selectiontest" selectiontest.cc)
target_link_libraries("selectiontest" "dunecommon")

add_executable("syncertest" syncertest.cc)
