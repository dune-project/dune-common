add_python_targets(common
  __init__
  compatibility
  deprecated            # deprecated 2.8
  module
  checkconfiguration
  hashit
  pickle                # deprecated 2.8
  project
  utility
)
dune_add_pybind11_module(NAME _common)
set_property(TARGET _common PROPERTY LINK_LIBRARIES dunecommon)
