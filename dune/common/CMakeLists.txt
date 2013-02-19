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
