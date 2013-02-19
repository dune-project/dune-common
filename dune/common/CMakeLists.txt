add_subdirectory("parallel")
add_subdirectory("test")

add_library("dunecommon"
	exceptions.cc
	fmatrixev.cc
	ios_state.cc
	parametertree.cc
	parametertreeparser.cc
	path.cc
	stdstreams.cc)
set_target_properties(dunecommon PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
