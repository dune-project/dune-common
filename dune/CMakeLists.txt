add_subdirectory(common)

# if Python bindings are enabled, include necessary sub directories.
if(DUNE_ENABLE_PYTHONBINDINGS)
  add_subdirectory(python)
else()
  exclude_subdir_from_headercheck(python)
endif()
