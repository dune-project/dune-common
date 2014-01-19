# Searches for boost and provides the following function:
#
# add_dune_boost_flags(targets [LINK_ALL_BOOST_LIBRARIES]
#                      LIBRARIES lib1 lib2 ...)
#
# Adds boost compiler flags and libraries, and activates
# Boost for the specified targets. Libraries can either be
# provided by listing them after the LIBRARIES keyword or one
# can request linking with all available boost libraries by
# passing the LINK_ALL_BOOST_LIBRARIES option to the function
# call.
find_package(Boost)
set(HAVE_DUNE_BOOST ${Boost_FOUND})

#add all boost realted flags to ALL_PKG_FLAGS, this must happen regardless of a target using add_dune_boost_flags
if(HAVE_DUNE_BOOST)
  set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-DENABLE_BOOST=1")
  foreach(dir ${Boost_INCLUDE_DIRS})
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-I${dir}")
  endforeach()
endif()

function(add_dune_boost_flags _targets)
  cmake_parse_arguments(DUNE_BOOST LINK_ALL_BOOST_LIBRARIES "" LIBRARIES ${ARGN})
  if(Boost_FOUND)
    include_directories("${Boost_INCLUDE_DIRS}")
    foreach(_target ${_targets})
      if(DUNE_BOOST_LINK_ALL_BOOST_LIBRARIES)
	target_link_libraries(${_target} "${DUNE_Boost_LIBRARIES}")
      else(DUNE_BOOST_LINK_ALL_BOOST_LIBRARIES)
	target_link_libraries(${_target} "${DUNE_BOOST_LIBRARIES}")
      endif(DUNE_BOOST_LINK_ALL_BOOST_LIBRARIES)
      # The definitions are a hack as we do not seem to know which MPI implementation was
      # found.
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
	"${_props} -DENABLE_BOOST=1")
    endforeach(_target)
  endif(Boost_FOUND)
endfunction(add_dune_boost_flags)