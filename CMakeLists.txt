add_python_targets(common
  __init__
  compatibility
  module
  checkconfiguration
  hashit
  pickle
  project
)

dune_add_pybind11_module(NAME _common)
