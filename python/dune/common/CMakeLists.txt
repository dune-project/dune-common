add_python_targets(common
  __init__
  compatibility
  checkconfiguration
  hashit
  _common
  deprecated            # deprecated 2.8
  pickle                # deprecated 2.8
  module                # deprecated 2.8
  project               # deprecated 2.8
  utility               # deprecated 2.8
)
dune_symlink_to_source_files(FILES
  _common.cc
)
