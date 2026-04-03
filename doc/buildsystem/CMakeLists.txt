# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Install the buildsystem documentation defined in dune-common
install(FILES
  buildsystem-overview.rst
  buildsystem-faq.rst
  commands/add_latex_document.rst
  modules/UseLatexMk.rst
  variables/LATEXMK_PARANOID.rst
  DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Also always build the CMake API documentation in dune-common
dune_cmake_sphinx_doc(
  RST_SOURCES
    ${PROJECT_SOURCE_DIR}/cmake/scripts/index.rst.in
    ${CMAKE_CURRENT_SOURCE_DIR}/buildsystem-overview.rst
    ${CMAKE_CURRENT_SOURCE_DIR}/buildsystem-faq.rst
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/add_latex_document.rst
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/UseLatexMk.rst
    ${CMAKE_CURRENT_SOURCE_DIR}/variables/LATEXMK_PARANOID.rst
)
