add_executable(mpi_collective_benchmark EXCLUDE_FROM_ALL mpi_collective_benchmark.cc)
target_link_libraries(mpi_collective_benchmark PRIVATE Dune::dune-common)

configure_file(options.ini options.ini COPYONLY)
