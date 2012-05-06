find_package(Boost)

message("Boost_LIBRARIES=${Boost_LIBRARIES}")
message("Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")

function(add_dune_boost_flags _targets)
  if(Boost_FOUND)
    include_directories("${Boost_INCLUDE_DIRS}")
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${Boost_LIBRARIES})
      # The definitions are a hack as we do not seem to know which MPI implementation was
      # found.
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
	"${_props} -DENABLE_BOOST=1")
    endforeach(_target)
  endif(Boost_FOUND)
endfunction(add_dune_boost_flags)
