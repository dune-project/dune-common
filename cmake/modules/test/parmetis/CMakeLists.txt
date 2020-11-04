cmake_minimum_required(VERSION 3.13)
project(parmetistest LANGUAGES C CXX)

find_package(ParMETIS)
if(ParMETIS_FOUND)
  add_executable(parmetis parmetistest.cc)
  target_link_libraries(parmetis PUBLIC ParMETIS::ParMETIS)

  set(procnum 3)
  set(exe ${CMAKE_CURRENT_BINARY_DIR}/parmetis)

  enable_testing()
  add_test(NAME parmetistest
    COMMAND ${MPIEXEC} ${MPIEXEC_PREFLAGS} ${MPIEXEC_NUMPROC_FLAG} ${procnum} ${exe} ${MPIEXEC_POSTFLAGS})
endif()
