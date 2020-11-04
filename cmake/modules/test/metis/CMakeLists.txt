cmake_minimum_required(VERSION 3.13)
project(metistest LANGUAGES CXX)

find_package(METIS)
if(METIS_FOUND)
  add_executable(metis metistest.cc)
  target_link_libraries(metis PUBLIC METIS::METIS)

  enable_testing()
  add_test(metistest metis)
endif()
