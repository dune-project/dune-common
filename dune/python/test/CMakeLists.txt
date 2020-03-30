dune_python_add_test(NAME pythontests
                     COMMAND ${PYTHON_EXECUTABLE} pythontests.py
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     LABELS quick)
