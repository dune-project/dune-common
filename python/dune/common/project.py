# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import os

def write_cmake_file(dir, content=None, subdirs=None, install=None):
    if not os.path.isdir(dir):
        raise ValueError('Directory \'' + dir + '\' does not exist.')

    append = []
    if subdirs is not None:
        append += ['add_subdirectory("' + d + '")' for d in subdirs]

    if install is not None and install[0]:
        append += ['install(FILES'] + ['  ' + f for f in install[0]] + ['  DESTINATION "' + install[1] + '")']

    with open(os.path.join(dir, 'CMakeLists.txt'), 'w') as file:
        if content is not None:
            file.write('\n'.join(content) + '\n')
        if append:
            file.write('\n'.join(append) + '\n')


def write_config_h_cmake(dir, project, public_content=None):
    if not os.path.isdir(dir):
        raise ValueError('Directory \'' + dir + '\' does not exist.')
    project_up = project.upper().replace('-', '_')
    with open(os.path.join(dir, 'config.h.cmake'), 'w') as file:
        file.write('/* begin ' + project + '\n   put the definitions for config.h specific to\n   your project here. Everything above will be\n   overwritten\n*/\n\n')
        file.write('/* begin private */\n')
        file.write('\n/* Name of package */\n#define PACKAGE "${DUNE_MOD_NAME}"\n')
        file.write('\n/* Define to the full name of this package */\n#define PACKAGE_NAME "${DUNE_MOD_NAME}"\n')
        file.write('\n/* Define to the version of this package */\n#define PACKAGE_VERSION "${DUNE_MOD_VERSION}"\n')
        file.write('\n/* Define to the full name and version of this package */\n#define PACKAGE_STRING "${DUNE_MOD_NAME} ${DUNE_MOD_VERSION}"\n')
        file.write('\n/* Define to the address where bug reports for this package should be sent */\n#define PACKAGE_BUGREPORT "${DUNE_MAINTAINER}"\n')
        file.write('\n/* Define to the one symbol short name of this package */\n#define PACKAGE_TARNAME "${DUNE_MOD_NAME}"\n')
        file.write('\n/* Define to the home page for this package */\n#define PACKAGE_URL "${DUNE_MOD_URL}"\n')
        file.write('\n/* end private */\n')
        file.write('\n/* Define to the version of ' + project + ' */\n#define ' + project_up + '_VERSION "${' + project_up + '_VERSION}"\n')
        file.write('\n/* Define to the major version of ' + project + ' */\n#define ' + project_up + '_VERSION_MAJOR "${' + project_up + '_VERSION_MAJOR}"\n')
        file.write('\n/* Define to the minor version of ' + project + ' */\n#define ' + project_up + '_VERSION_MINOR "${' + project_up + '_VERSION_MINOR}"\n')
        file.write('\n/* Define to the revision of ' + project + ' */\n#define ' + project_up + '_VERSION_REVISION "${' + project_up + '_VERSION_REVISION}"\n')
        if public_content is not None:
            file.write('\n'.join(public_content) + '\n\n')
        file.write('\n/* end ' + project + '\n   Everything below here will be overwritten.\n*/\n')


def make_cmake_modules(dir, description, macros):
    if not os.path.isdir(dir):
        raise ValueError('Directory \'' + dir + '\' does not exist.')

    cmake_dir_rel = os.path.join('cmake', 'modules')
    cmake_dir = os.path.join(dir, cmake_dir_rel)
    if not os.path.isdir(cmake_dir):
        os.makedirs(cmake_dir)

    macroFileName = ''.join([word[0].upper() + word[1:] for word in description.name.split('-')]) + 'Macros.cmake'
    write_cmake_file(cmake_dir, install=([macroFileName], '${DUNE_INSTALL_MODULEDIR}'))

    with open(os.path.join(cmake_dir, macroFileName), 'w') as file:
        file.write('\n'.join(macros) + '\n')
    return cmake_dir_rel


def make_project(dir, description, subdirs=None, enable_all_packages=True, is_dunepy=False):
    if not os.path.isdir(dir):
        raise ValueError('Directory \'' + dir + '\' does not exist.')
    with open(os.path.join(dir, 'dune.module'), 'w') as file:
        file.write(repr(description))

    cmake_content = ['cmake_minimum_required(VERSION 3.13)', 'project(' + description.name + ' C CXX)']
    if is_dunepy:
        cmake_content += ['', 'set(DUNE_ENABLE_PYTHONBINDINGS ON)']
    cmake_content += ['',
                      'if(NOT (dune-common_DIR OR dune-common_ROOT OR "${CMAKE_PREFIX_PATH}" MATCHES ".*dune-common.*"))',
                      '  string(REPLACE ${PROJECT_NAME} dune-common dune-common_DIR ${PROJECT_BINARY_DIR})',
                      'endif()']
    cmake_content += ['', 'find_package(dune-common REQUIRED)']
    if subdirs is not None and 'cmake/modules' in subdirs:
        cmake_content += ['list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules" "${dune-common_MODULE_PATH}")']
    else:
        cmake_content += ['list(APPEND CMAKE_MODULE_PATH ${dune-common_MODULE_PATH})']
    cmake_content += ['', 'include(DuneMacros)', 'dune_project()']
    if enable_all_packages:
        cmake_content += ['dune_enable_all_packages()']
    if subdirs is not None:
        cmake_content += [''] + ['add_subdirectory("' + d + '")' for d in subdirs]
    cmake_content += ['', 'finalize_dune_project(GENERATE_CONFIG_H_CMAKE)']
    write_cmake_file(dir, cmake_content)

    with open(os.path.join(dir, description.name + '.pc.in'), 'w') as file:
        file.write('prefix=@prefix@\nexec_prefix=@exec_prefix@\n')
        file.write('libdir=@libdir@\nincludedir=@includedir@\n')
        file.write('CXX=@CXX@\nCC=@CC@\n')
        file.write('DEPENDENCIES=@REQUIRES@\n\n')
        file.write('Name: @PACKAGE_NAME@\n')
        file.write('Version: @VERSION@\n')
        file.write('Description: DUNE module "' + description.name + '"\n')
        file.write('URL: http://dune-project.org\n')
        file.write('Requires: ${DEPENDENCIES}\n')
        file.write('Libs: -L${libdir}\n')
        file.write('Cflags: -I${includedir}\n')

    write_config_h_cmake(dir, description.name)
