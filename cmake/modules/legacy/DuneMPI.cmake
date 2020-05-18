# text for feature summary
set_package_properties("MPI" PROPERTIES
  DESCRIPTION "Message Passing Interface library"
  PURPOSE "Parallel programming on multiple processors")

# adds MPI flags to the targets
function (add_dune_mpi_flags TARGET)
  target_link_libraries(${TARGET}
    $<TARGET_NAME_IF_EXISTS:MPI::MPI_CXX>)
endfunction (add_dune_mpi_flags)
