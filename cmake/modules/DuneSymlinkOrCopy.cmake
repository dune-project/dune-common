# This module provides convenience macros to provide files from the source tree in the build tree.
#
# It provides the following macros:
#
#   dune_add_copy_command(filename)
#
# This macro adds a file-copy command.
# The file_name is the name of a file that exists
# in the source tree. This file will be copied
# to the build tree when executing this command.
# Notice that this does not create a top-level
# target. In order to do this you have to additionally
# call add_custom_target(...) with dependency
# on the file.
#
#   dune_add_copy_target(target_name file_name)
#
# This macro adds a file-copy target under given target_name.
# The file_name is the name of a file that exists
# in the source tree. This file will be copied
# to the build tree.
#
#   dune_add_copy_dependency(target file_name)
#
# This macro adds a copy-dependecy to a target
# The file_name is the name of a file that exists
# in the source tree. This file will be copied
# to the build tree.
#
#
# .. cmake_function:: dune_add_copy_command
#
#    .. cmake_param:: filename
#       :positional:
#       :single:
#       :required:
#
#    TODO DOC ME!
#
# .. cmake_function:: dune_add_copy_target
#
#    .. cmake_param:: target_name
#       :positional:
#       :single:
#       :required:
#
#    .. cmake_param:: filename
#       :positional:
#       :single:
#       :required:
#
#    TODO DOC ME!
#
# .. cmake_function:: dune_add_copy_dependency
#
#    .. cmake_param:: target
#       :positional:
#       :single:
#       :required:
#
#    .. cmake_param:: filename
#       :positional:
#       :single:
#       :required:
#
#    TODO DOC ME!
#
# .. cmake_function:: dune_symlink_to_source_tree
#
#    .. cmake_param:: NAME
#       :single:
#
#       The name of the symlink, defaults to :code:`src_dir`.
#
#    This function will place a symlink into every subdirectory
#    of the build tree, that allows to jump to the corresponding
#    source directory. Call this from your top-level :code:`CMakeLists.txt`
#    to enable it for a given module. To enable it for all modules,
#    set the variable :ref:`DUNE_SYMLINK_TO_SOURCE_TREE` instead.
#    If used on Windows systems, a warning is issued.
#
# .. cmake_variable:: DUNE_SYMLINK_TO_SOURCE_TREE
#
#    If this variable is set to TRUE, the functionality of
#    :ref:`dune_symlink_to_source_tree` is enabled in all modules.
#    This will place symlinks to the corresponding source directory
#    in every subdirectory of the build directory.
#
# .. cmake_function:: dune_symlink_to_source_files
#
#    .. cmake_param:: FILES
#       :multi:
#       :required:
#
#       The list of files to symlink.
#
#    Create symlinks in the current build directory, which
#    point to files in the source directory. This is usually
#    used for grid and ini files and the like. On Windows systems,
#    a warning is issued and copying is used as a fallback to
#    symlinking.
#

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
  # parse arguments
  include(CMakeParseArguments)
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
    file(GLOB_RECURSE files RELATIVE ${CMAKE_SOURCE_DIR} "*CMakeLists.txt")

    # iterate over all files, extract the directory name and write a symlink in the corresponding build directory
    foreach(f ${files})
      get_filename_component(dir ${f} DIRECTORY)
      if(NOT "${CMAKE_SOURCE_DIR}/${dir}" MATCHES "${CMAKE_BINARY_DIR}/*")
        execute_process(COMMAND ${CMAKE_COMMAND} "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/${dir}" "${CMAKE_BINARY_DIR}/${dir}/${ARG_NAME}")
      endif(NOT "${CMAKE_SOURCE_DIR}/${dir}" MATCHES "${CMAKE_BINARY_DIR}/*")
    endforeach()
  endif()
endfunction(dune_symlink_to_source_tree)

function(dune_symlink_to_source_files)
  # parse arguments
  include(CMakeParseArguments)
  cmake_parse_arguments(ARG "" "" "FILES" ${ARGN})
  if(ARG_UNPARSED_ARGUMENTS)
    message(WARNING "You are using dune_symlink_to_source_files without named arguments (or have typos in your named arguments)!")
  endif()

  # create symlinks for all given files
  foreach(f ${ARG_FILES})
    # check for Windows to issue a warning
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      if(NOT DEFINED DUNE_WINDOWS_SYMLINK_WARNING)
        message(WARNING "Your module wanted to create symlinks, but you cannot do that on your platform.")
        set(DUNE_WINDOWS_SYMLINK_WARNING)
      endif()
      dune_add_copy_command(${f})
    else()
      # create symlink
      execute_process(COMMAND ${CMAKE_COMMAND} "-E" "create_symlink" "${CMAKE_CURRENT_SOURCE_DIR}/${f}" "${CMAKE_CURRENT_BINARY_DIR}/${f}")
    endif()
  endforeach()
endfunction(dune_symlink_to_source_files)
