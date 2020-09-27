execute_process(COMMAND
  ${DOXYGEN_EXECUTABLE} Doxyfile OUTPUT_FILE doxygen.log ERROR_FILE doxygen.log
  TIMEOUT 3600)
