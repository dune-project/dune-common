# Module that provides a custom target make doc at the top level
# directory and utility macros for creating install directives
# that make sure that the files to be installed are previously
# generated even if make doc was not called.
#
add_custom_target(doc)

MACRO(create_doc_install filename targetdir)
  get_filename_component(targetfile ${filename} NAME)
  set(install_command ${CMAKE_COMMAND} -D FILE=${filename} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/InstallFile.cmake)
  # create a custom target for the installation
  add_custom_target(install_${targetfile} ${install_command}
    COMMENT "Installing ${filename} to ${targetdir}"
    DEPENDS ${filename})
  # When installing call cmake install with the above install target
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )")
ENDMACRO(create_doc_install)
