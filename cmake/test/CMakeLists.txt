# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

dune_declare_test_label(LABELS buildsystem)

add_test(
    NAME dunepolicytest
    COMMAND ${CMAKE_COMMAND}
    -DCMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/cmake/modules
    -S ${CMAKE_CURRENT_SOURCE_DIR}/dune_policy
    -B ${CMAKE_CURRENT_BINARY_DIR}/dune_policy
)
set_tests_properties(dunepolicytest PROPERTIES LABELS buildsystem)
