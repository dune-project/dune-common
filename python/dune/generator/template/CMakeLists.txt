add_subdirectory(python)

set(templatefiles CMakeLists.txt.template dune.module.template config.h.cmake dune-py.pc.in)
dune_symlink_to_source_files(FILES ${templatefiles})
if (SKBUILD)
  install(FILES ${templatefiles} DESTINATION python/dune/generator/template)
endif()
