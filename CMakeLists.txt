add_python_targets(common
  __init__
  module
  checkconfiguration
  project
)

add_library(_common SHARED _common.cc)
set_target_properties(_common PROPERTIES PREFIX "")
