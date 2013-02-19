add_subdirectory("parallel")
add_subdirectory("test" EXCLUDE_FROM_ALL)

#build the library dunecommon
add_library("dunecommon"
	exceptions.cc
	fmatrixev.cc
	ios_state.cc
	parametertree.cc
	parametertreeparser.cc
	path.cc
	stdstreams.cc)
target_link_libraries("dunecommon" ${LAPACK_LIBRARIES})

install(TARGETS dunecommon  EXPORT DuneCommonTargets DESTINATION lib)
install(EXPORT DuneCommonTargets
  DESTINATION lib/cmake)
# build the library in lib
set_target_properties(dunecommon PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set_target_properties(dunecommon PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

#install headers
install(FILES
        alignment.hh
	array.hh
	arraylist.hh
	bartonnackmanifcheck.hh
	bigunsignedint.hh
	binaryfunctions.hh
	bitsetvector.hh
	classname.hh
	collectivecommunication.hh
	debugstream.hh
	deprecated.hh
	densematrix.hh
	densevector.hh
	documentation.hh
	dynmatrix.hh
	dynvector.hh
	enumset.hh
	exceptions.hh
	fassign.hh
	finitestack.hh
	float_cmp.cc
	float_cmp.hh
	fmatrix.hh
	fmatrixev.hh
	forloop.hh
	ftraits.hh
	function.hh
	fvector.hh
	gcd.hh
	genericiterator.hh
	geometrytype.hh
	geometrytypeindex.hh
	gmpfield.hh
	indent.hh
	interfaces.hh
	ios_state.hh
	iteratorfacades.hh
	lcm.hh
	lru.hh
	math.hh
	matvectraits.hh
	misc.hh
	mpicollectivecommunication.hh
	mpiguard.hh
	mpihelper.hh
	mpitraits.hh
	nullptr.hh
	parametertree.hh
	parametertreeparser.hh
	path.hh
	poolallocator.hh
	precision.hh
	propertymap.hh
	reservedvector.hh
	shared_ptr.hh
	singleton.hh
	sllist.hh
	static_assert.hh
	stdstreams.hh
	timer.hh
	tuples.hh
	tupleutility.hh
	typetraits.hh
	unused.hh
	version.hh
DESTINATION include/dune/common)
