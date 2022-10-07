# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#add_executable(poosc08 "poosc08.cc")
#target_link_libraries(poosc08 PUBLIC "dunecommon")

#add_executable(poosc08_test "poosc08_test.cc")
#target_link_libraries(poosc08_test PUBLIC "dunecommon")

#add_executable(indexset "indexset.cc")
#target_link_libraries(indexset PUBLIC "dunecommon")

#add_dune_mpi_flags("poosc08;poosc08_test;indexset")

dune_add_latex_document(
  SOURCE communication.tex
  FATHER_TARGET doc
  BUILD_ON_INSTALL
  INSTALL ${CMAKE_INSTALL_DOCDIR}/comm)
