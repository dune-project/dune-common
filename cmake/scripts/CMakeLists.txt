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
  WritePythonCMakeMetadata.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)

# Install executable programs
install(PROGRAMS
  extract_cmake_data.py
  run-in-dune-env.sh.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts
)
