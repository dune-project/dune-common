# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(modules)
add_subdirectory(scripts)
if(DUNE_MODULE_ENABLE_TESTING)
  add_subdirectory(test)
endif()
