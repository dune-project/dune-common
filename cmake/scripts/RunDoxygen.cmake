# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

execute_process(COMMAND
  ${DOXYGEN_EXECUTABLE} Doxyfile OUTPUT_FILE doxygen.log ERROR_FILE doxygen.log
  TIMEOUT 3600)
