# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(common)
add_subdirectory(generator)
add_subdirectory(typeregistry)

add_python_targets(dune
  __main__
  commands
  create
  packagemetadata
  plotting
  deprecate
  utility
)
