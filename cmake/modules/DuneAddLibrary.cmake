function (dune_add_library NAME)
  include(DuneModuleName)
  dune_module_name(${NAME} MODULE_NAME)

  add_library(${NAME})
  add_library(Dune::${NAME} ALIAS ${NAME})

  # set properties of target ${NAME}
  set_target_properties(${NAME} PROPERTIES
    EXPORT_NAME ${NAME}
    VERSION ${${NAME}_VERSION}
    SOVERSION ${${NAME}_VERSION_MAJOR} )

  # set include directories for target
  target_include_directories(${NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )

  # set <PackageName>_FOUND as if find_package was called for this library
  set(${NAME}_FOUND ON PARENT_SCOPE)
endfunction(dune_add_library)
