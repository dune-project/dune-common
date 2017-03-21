# CMake does horrible things to lists passed into scripts as arguments:
# They become space-separated strings. To restore the list-nature we have
# to replace the spaces with semicolons again (CMakes way of "implementing"
# lists)... Urgh!
string(REPLACE " " ";" CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
string(REPLACE " " ";" CMDLINE ${CMDLINE})

include(DuneExecuteProcess)
dune_execute_process(COMMAND ${CMDLINE}
                     ERROR_MESSAGE "Cannot install python package at ${PACKAGE_PATH}!")
