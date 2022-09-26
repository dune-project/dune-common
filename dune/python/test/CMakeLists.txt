# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

dune_python_add_test(NAME pythontests
                     SCRIPT pythontests.py
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     MPI_RANKS 1 2 4
                     LABELS quick
)

dune_add_test(SOURCES test_embed1.cc
              LINK_LIBRARIES ${DUNE_LIBS} ${Python3_LIBRARIES}
              LABELS quick
              COMMAND ${CMAKE_BINARY_DIR}/run-in-dune-env
              CMD_ARGS $<TARGET_FILE:test_embed1>
)
target_compile_definitions(test_embed1 PRIVATE PYTHON_INTERPRETER=L"${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}")

dune_add_test(SOURCES test_embed2.cc
              LINK_LIBRARIES ${DUNE_LIBS} ${Python3_LIBRARIES}
              LABELS quick
              COMMAND ${CMAKE_BINARY_DIR}/run-in-dune-env
              CMD_ARGS $<TARGET_FILE:test_embed2>
)
target_compile_definitions(test_embed2 PRIVATE PYTHON_INTERPRETER=L"${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}")
