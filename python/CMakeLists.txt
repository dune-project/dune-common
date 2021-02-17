add_subdirectory(dune)

configure_file(MANIFEST.in MANIFEST.in)
configure_file(setup.py.in setup.py)
configure_file(pyproject.toml.in pyproject.toml)

dune_python_install_package(
  PATH "."
  DEPENDS _common _typeregistry
  CMAKE_METADATA_FILE dune/common/metadata.cmake
)
