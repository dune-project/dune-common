# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Install non-executable scripts
install(FILES
  conf.py.in
  CreateDoxyFile.cmake
  venvpath.py
  checkvenvconf.py
  FinalizeHeadercheck.cmake
  index.rst.in
  InstallFile.cmake
  main77.cc.in
  module_library.cc.in
  pyversion.py
  RunDoxygen.cmake
  setup.py.in
  sphinx_cmake_dune.py
  sphinx_moderncmakedomain.py
  sphinx_moderncmakedomain.py.license
  WritePythonCMakeMetadata.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)

# Install executable programs
install(PROGRAMS
  extract_cmake_data.py
  generate_cmake_command_pages.py
  run-in-dune-env.sh.in
  set-dune-pythonpath.sh.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts
)
