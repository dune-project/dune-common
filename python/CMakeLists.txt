add_subdirectory(dune)

# installing the python package mpi4py fails if mpi is not available on the
# system so we only add it here to the python requires in setup.py if mpi was found
if(MPI_C_FOUND)
    set(ProjectPythonRequires "${ProjectPythonRequires} mpi4py")
endif()

configure_file(MANIFEST.in MANIFEST.in)
configure_file(setup.py.in setup.py)
configure_file(pyproject.toml.in pyproject.toml)

dune_python_install_package(
  PATH "."
  DEPENDS _common _typeregistry
  CMAKE_METADATA_FILE dune/common/metadata.cmake
  FLAGS CMAKE_BUILD_TYPE CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_C_COMPILER CMAKE_C_FLAGS CMAKE_PREFIX_PATH
)
