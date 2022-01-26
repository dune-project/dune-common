add_subdirectory(common)
add_subdirectory(generator)
add_subdirectory(typeregistry)

add_python_targets(dune
  __main__
  commands
  create
  packagemetadata
  plotting
  deprecate
  utility
)
