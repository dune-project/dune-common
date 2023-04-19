# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(python)

set(templatefiles CMakeLists.txt.template dune.module.template config.h.cmake dune-py.pc.in dune-compiler_launcher.sh compiler_launcher.sh.template)
dune_symlink_to_source_files(FILES ${templatefiles})
if (SKBUILD)
  install(FILES ${templatefiles} DESTINATION python/dune/generator/template)
endif()
