dune_symlink_to_source_files(FILES myclass.hh test_class_export.py)

dune_python_add_test(NAME combinedtests
                     COMMAND ${PYTHON_EXECUTABLE} combinedtests.py
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     LABELS quick)
