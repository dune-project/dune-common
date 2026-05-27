# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

if (NOT DEFINED DOXYFILE)
  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
endif()

execute_process(COMMAND
  ${DOXYGEN_EXECUTABLE} ${DOXYFILE} OUTPUT_FILE doxygen.log ERROR_FILE doxygen.log
  TIMEOUT 3600)
