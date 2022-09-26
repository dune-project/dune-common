# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(template)

add_python_targets(generator
  __init__
  algorithm
  importclass
  cmakebuilder
  exceptions
  generator
  remove
  )
