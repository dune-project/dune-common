# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

dune_declare_test_label(LABELS buildsystem)

# test that dune policies are local to a project
add_test(
    NAME dunepolicytest
    COMMAND ${CMAKE_COMMAND}
    -DCMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/cmake/modules
    -S ${CMAKE_CURRENT_SOURCE_DIR}/dune_policy
    -B ${CMAKE_CURRENT_BINARY_DIR}/dune_policy
)

# test that a stand alone project depending on dune-common can be configured
add_test(
    NAME dunecommonstandaloneconfiguretest
    COMMAND ${CMAKE_COMMAND}
    -DCMAKE_PREFIX_PATH=${PROJECT_BINARY_DIR}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/dune_common_standalone
    -B ${CMAKE_CURRENT_BINARY_DIR}/dune_common_standalone
)
# test that a standalone project depending on dune-common can be built
add_test(
    NAME dunecommonstandalonebuildtest
    COMMAND ${CMAKE_COMMAND}
    --build ${CMAKE_CURRENT_BINARY_DIR}/dune_common_standalone
)

set_tests_properties(dunecommonstandalonebuildtest PROPERTIES DEPENDS dunecommonstandaloneconfiguretest)

set_tests_properties(
        dunepolicytest dunecommonstandaloneconfiguretest dunecommonstandalonebuildtest
    PROPERTIES LABELS buildsystem
)
