# this is script is called at the end of all header checks
if(ENABLE_HEADERCHECK)
  message("Headerchecks finished! Rerun CMake if a new file has not been checked!")
else()
  message("The headercheck feature is currently disabled. You can enable it by adding ENABLE_HEADERCHECK=1 to your cmake flags.")
endif()

#message("Running make clean on headercheck targets...")
#this cleans the build directory from pollution through headerchecks but prevents caching... :/
#file(GLOB_RECURSE list "./CMakeFiles/headercheck_*/cmake_clean.cmake")
#foreach(item ${list})
 # execute_process(COMMAND ${CMAKE_COMMAND} -P ${item})
#endforeach()