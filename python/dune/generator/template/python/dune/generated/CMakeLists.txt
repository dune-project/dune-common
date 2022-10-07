# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

set(templatefiles CMakeLists.txt.template extractCompiler.cc)
dune_symlink_to_source_files(FILES ${templatefiles})
if (SKBUILD)
  install(FILES ${templatefiles} DESTINATION python/dune/generator/template/python/dune/generated)
endif()
