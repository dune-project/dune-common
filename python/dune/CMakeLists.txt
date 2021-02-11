add_subdirectory(common)
add_subdirectory(generator)
add_subdirectory(typeregistry)

add_python_targets(dune
  __init__
  __main__
  create
  packagemetadata
  plotting
  deprecate
  utility
)
