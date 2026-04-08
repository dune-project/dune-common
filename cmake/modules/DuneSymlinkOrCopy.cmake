# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneSymlinkOrCopy
=================

Convenience commands to expose files from the source tree inside the build
tree by copying or symlinking them.

.. cmake:command:: dune_add_copy_command

  Add a custom command that copies a source-tree file into the current build
  directory.

  .. code-block:: cmake

    dune_add_copy_command(<filename>)

  ``filename``
    Path to a file relative to the current source directory.

  This command only creates the custom command for the copied output file. It
  does not add a top-level target by itself.

.. cmake:command:: dune_add_copy_target

  Add a custom target that copies a source-tree file into the current build
  directory.

  .. code-block:: cmake

    dune_add_copy_target(<target-name> <filename>)

  ``target-name``
    Name of the custom target to create.

  ``filename``
    Path to a file relative to the current source directory.

  The created target depends on the copied file output created by
  :cmake:command:`dune_add_copy_command()`.

.. cmake:command:: dune_add_copy_dependency

  Add a file copy step as a dependency of an existing target.

  .. code-block:: cmake

    dune_add_copy_dependency(<target> <filename>)

  ``target``
    Existing target that should depend on the copied file.

  ``filename``
    Path to a file relative to the current source directory.

  This command creates an auxiliary copy target and adds it as a dependency of
  ``target``.

.. cmake:command:: dune_symlink_to_source_tree

  Create directory symlinks from the build tree back into the source tree.

  .. code-block:: cmake

    dune_symlink_to_source_tree([NAME <link-name>])

  ``NAME``
    Name of the symlink created in each build directory. The default is
    ``src_dir``.

  Call this from a top-level ``CMakeLists.txt`` to create a symlink in every
  build-tree subdirectory that points to the corresponding source directory. On
  Windows, a warning is issued instead.

.. cmake:variable:: DUNE_SYMLINK_TO_SOURCE_TREE

  If set to ``TRUE``, the functionality of
  :cmake:command:`dune_symlink_to_source_tree()` is enabled globally for a
  module.

.. cmake:variable:: DUNE_SYMLINK_RELATIVE_LINKS

  If set to ``TRUE``, relative symlinks are created instead of absolute ones.

.. cmake:command:: dune_symlink_to_source_files

  Create symlinks, or copies on Windows, for selected source files in the
  current build directory.

  .. code-block:: cmake

    dune_symlink_to_source_files(
      FILES <files>...
      [DESTINATION <relative-path>]
    )

  ``FILES``
    Files relative to the current source directory that should be mirrored into
    the build directory.

  ``DESTINATION``
    Relative destination directory below the current build directory.

  On Windows, copying is used as a fallback because symlink creation is not
  generally available.

#]=======================================================================]
include_guard(GLOBAL)

macro(dune_add_copy_command file_name)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${file_name}"
        COMMAND    ${CMAKE_COMMAND}
        ARGS       -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${file_name}" "${file_name}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file_name}"
        )
endmacro(dune_add_copy_command file_name)

macro(dune_add_copy_target target_name file_name)
    dune_add_copy_command(${file_name})
    add_custom_target("${target_name}" ALL DEPENDS "${file_name}")
endmacro(dune_add_copy_target target_name file_name)

macro(dune_add_copy_dependency target file_name)
    message(STATUS "Adding copy-to-build-dir dependency for ${file_name} to target ${target}")
    dune_add_copy_target("${target}_copy_${file_name}" "${file_name}")
    add_dependencies(${target} "${target}_copy_${file_name}")
endmacro(dune_add_copy_dependency)

function(dune_symlink_to_source_tree)
  # if source and binary dir are equal then the symlink will create serious problems
  if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    return()
  endif()

  # parse arguments
  cmake_parse_arguments(ARG "" "NAME" "" ${ARGN})
  if(NOT ARG_NAME)
    set(ARG_NAME "src_dir")
  endif()

  # check for Windows to issue a warning
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if(NOT DEFINED DUNE_WINDOWS_SYMLINK_WARNING)
      message(WARNING "Your module wanted to create symlinks, but you cannot do that on your platform.")
      set(DUNE_WINDOWS_SYMLINK_WARNING)
    endif()
  else()
    # get a list of all files in the current source directory and below.
    file(GLOB_RECURSE files RELATIVE ${PROJECT_SOURCE_DIR} "*CMakeLists.txt")

    # iterate over all files, extract the directory name and write a symlink in the corresponding build directory
    foreach(f ${files})
      get_filename_component(dir ${f} DIRECTORY)
      set(_target "${PROJECT_SOURCE_DIR}/${dir}")
      if(DUNE_SYMLINK_RELATIVE_LINKS)
        file(RELATIVE_PATH _target "${PROJECT_BINARY_DIR}/${dir}" "${_target}")
      endif()
      execute_process(COMMAND ${CMAKE_COMMAND} "-E" "create_symlink" "${_target}" "${PROJECT_BINARY_DIR}/${dir}/${ARG_NAME}")
    endforeach()
  endif()
endfunction(dune_symlink_to_source_tree)

function(dune_symlink_to_source_files)

  # if source and binary dir are equal then the symlink will create serious problems
  if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    return()
  endif()

  # parse arguments
  cmake_parse_arguments(ARG "" "DESTINATION" "FILES" ${ARGN})
  if(ARG_UNPARSED_ARGUMENTS)
    message(WARNING "You are using dune_symlink_to_source_files without named arguments (or have typos in your named arguments)!")
  endif()

  # create symlinks for all given files
  foreach(f ${ARG_FILES})
    # check whether there is an explicitly given destination
    if(ARG_DESTINATION)
      set(destination "${ARG_DESTINATION}/")
    else()
      set(destination "")
    endif()
    # check for Windows to issue a warning
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      if(NOT DEFINED DUNE_WINDOWS_SYMLINK_WARNING)
        message(WARNING "Your module wanted to create symlinks, but you cannot do that on your platform.")
        set(DUNE_WINDOWS_SYMLINK_WARNING)
      endif()
      # create a copy
      execute_process(COMMAND ${CMAKE_COMMAND} "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/${f}" "${CMAKE_CURRENT_BINARY_DIR}/${destination}${f}")
    else()
      # create symlink
      set(_target "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
      if(DUNE_SYMLINK_RELATIVE_LINKS)
       file(RELATIVE_PATH _target "${CMAKE_CURRENT_BINARY_DIR}/${destination}" "${_target}")
      endif()
      execute_process(COMMAND ${CMAKE_COMMAND} "-E" "create_symlink" "${_target}" "${CMAKE_CURRENT_BINARY_DIR}/${destination}${f}")
    endif()
  endforeach()
endfunction(dune_symlink_to_source_files)
