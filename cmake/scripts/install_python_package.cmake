string(REPLACE " " ";" CMDLINE ${CMDLINE})

include(DuneExecuteProcess)
dune_execute_process(COMMAND ${CMDLINE}
                     ERROR_MESSAGE "Cannot install python package at ${PACKAGE_PATH}!")
