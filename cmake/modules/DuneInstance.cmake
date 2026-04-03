# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneInstance
============

Generate explicit template instantiations from CMake-driven template files.

This module helps split many explicit instantiations across generated
translation units. A typical workflow is:

.. code-block:: cmake

  dune_instance_begin(FILES mytest.cc mytest.hh)

  foreach(TYPE IN ITEMS bool char int double)
    dune_instance_add(ID "${TYPE}" FILES mytest_instance.cc)
  endforeach()

  dune_instance_end()

  list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\\.cc$]])
  dune_add_test(NAME mytest SOURCES ${DUNE_INSTANCE_GENERATED})

The files listed in :cmake:command:`dune_instance_begin()` are expected as
template files with an implicit ``.in`` suffix. Embedded templates delimited by
``@template@`` and ``@endtemplate@`` are instantiated by
:cmake:command:`dune_instance_add()`. The collected generated content is
written by :cmake:command:`dune_instance_end()`.

Template generation is based on :dune:cmake-command:`configure_file` and
``string(CONFIGURE ...)``, so current CMake variable values are substituted
directly.

.. cmake:command:: dune_instance_begin

  Prepare generation of a family of explicit instantiations.

  .. code-block:: cmake

    dune_instance_begin(FILES <file-spec>...)

  ``FILES``
    Template files with embedded templates. In the common case these are given
    without the trailing ``.in`` suffix.

  The command reads the given templates, extracts embedded template sections,
  and applies substitutions to the remaining content with the variables
  currently in scope.

  .. note::

    A matching :cmake:command:`dune_instance_end()` call is required. Since the
    implementation communicates through variables in the caller scope,
    begin/end blocks must not be nested in the same scope.

.. cmake:command:: dune_instance_add

  Instantiate embedded templates and generate instance files for one set of
  template arguments.

  .. code-block:: cmake

    dune_instance_add(
      FILES <file-spec>...
      ID <id>
      [TEMPLATE <template-name>]
    )

  ``FILES``
    Template file specifications. These are usually file names without the
    trailing ``.in`` suffix.

  ``ID``
    Unique identifier used to derive instance file names. The identifier is
    normalized by replacing non-alphanumeric characters with ``_`` before it is
    inserted into the generated file name.

  ``TEMPLATE``
    Name of the embedded template to instantiate. The default is the unnamed
    embedded template.

  Each file specification is mapped to a template file and an instance file
  name through :cmake:command:`dune_instance_from_id()`.

.. cmake:command:: dune_instance_end

  Finish a generation block started by :cmake:command:`dune_instance_begin()`.

  .. code-block:: cmake

    dune_instance_end()

  This writes the files prepared by :cmake:command:`dune_instance_begin()`,
  including all content contributed by :cmake:command:`dune_instance_add()`.

.. cmake:command:: dune_instance_parse_file_spec

  Parse a file specification into a template file name and an instance file
  name.

  .. code-block:: cmake

    dune_instance_parse_file_spec(<spec> <template-var> <instance-var>)

  ``spec``
    File specification to parse.

  ``template-var``
    Variable name receiving the template file name. It may be empty.

  ``instance-var``
    Variable name receiving the instance file name. It may be empty.

  A file specification can be written as ``template.in:instance`` or as a
  single name, in which case ``.in`` is appended or removed automatically.

.. cmake:command:: dune_instance_from_id

  Derive a template file name and a unique instance file name from a file
  specification and an identifier.

  .. code-block:: cmake

    dune_instance_from_id(<file-spec> <id> <template-var> <instance-var>)

  ``file-spec``
    File specification to interpret.

  ``id``
    Identifier used to derive the generated instance file name.

  ``template-var``
    Variable name receiving the template file name. It may be empty.

  ``instance-var``
    Variable name receiving the instance file name. It may be empty.

  The file specification is parsed by
  :cmake:command:`dune_instance_parse_file_spec()`. The derived base instance
  file name is then extended with a mangled form of ``id``.

.. cmake:command:: dune_instance_apply_bindir

  Make a file name relative to ``CMAKE_CURRENT_BINARY_DIR``.

  .. code-block:: cmake

    dune_instance_apply_bindir(<filename-var>)

  ``filename-var``
    Variable containing the file name to normalize.

  Relative paths are prefixed with ``${CMAKE_CURRENT_BINARY_DIR}``, matching
  the behavior of :dune:cmake-command:`configure_file`.

.. cmake:command:: dune_instance_generate_file

  Generate one file from one template with standard DUNE instance-generation
  substitutions.

  .. code-block:: cmake

    dune_instance_generate_file(<template> <instance>)

  ``TEMPLATE``
    Template file name.

  ``INSTANCE``
    Generated file name, interpreted relative to
    ``${CMAKE_CURRENT_BINARY_DIR}``.

  Besides generating the file, this command prepares convenience substitution
  variables such as ``TEMPLATE``, ``INSTANCE``, ``BINDIR_INSTANCE``, and
  ``GENERATED_SOURCE``. Re-generating the same output file with different
  content is treated as a fatal error.

.. cmake:variable:: DUNE_INSTANCE_GENERATED

  After :cmake:command:`dune_instance_end()`, this variable contains the list
  of generated files, including the implied ``${CMAKE_CURRENT_BINARY_DIR}``
  prefix where applicable.

  Do not modify this variable inside a
  :cmake:command:`dune_instance_begin()` /
  :cmake:command:`dune_instance_end()` block.

#]=======================================================================]
include_guard(GLOBAL)

# macro to print additional information to the cmake output file.
#
# .. deprecated:: 2.10
#
# Use `message(VERBOSE "message text")` instead. This macro will be
# removed after Dune 2.10.
macro(message_verbose TEXT)
  message(VERBOSE "${TEXT}")

  message(DEPRECATION "message_verbose is deprecated, use directly message(VERBOSE \"message text\")")
endmacro(message_verbose)


######################################################################
#
#  Coping with cmake list shortcomings
#

# We use these commands internally to quote text before adding it to lists as
# an element, and to unquote elements again after extracting them.  The quoted
# text is
# - free of ';' characters.  This avoids problems when using list(APPEND),
#   which does not quote ';' characters inside the appended element.  It would
#   also avoid problems with list(INSERT), which mangles any cmake quoting in
#   the list it inserts to, but we don't actually use that command.
# - free of '\' characters.  This avoids problems with a list element that
#   ends in a '\' merging with the next element, because the '\' quotes the
#   ';' that is used to separate the elements
# - non-empty.  This avoids the problem that cmake can't distinguish between
#   an empty list and a list with one empty element.
function(dune_instance_quote_element VAR)
  set(content "${${VAR}}")
  string(REPLACE [[$]] [[$s]] content "${content}")
  string(REPLACE [[;]] [[$:]] content "${content}")
  string(REPLACE [[\]] [[$/]] content "${content}")
  if(content STREQUAL "")
    set(content [[$@]])
  endif()
  set("${VAR}" "${content}" PARENT_SCOPE)
endfunction(dune_instance_quote_element)

function(dune_instance_unquote_element VAR)
  set(content "${${VAR}}")
  string(REPLACE [[$@]] [[]]  content "${content}")
  string(REPLACE [[$/]] [[\]] content "${content}")
  string(REPLACE [[$:]] [[;]] content "${content}")
  string(REPLACE [[$s]] [[$]] content "${content}")
  set("${VAR}" "${content}" PARENT_SCOPE)
endfunction(dune_instance_unquote_element)

######################################################################
#
#  instance name and template name manipulation
#

function(dune_instance_parse_file_spec spec template_var instance_var)
  string(REPLACE ":" ";" spec_items "${spec}")
  list(LENGTH spec_items len)

  # check arguments
  if(len GREATER 2)
    message(FATAL_ERROR "too many items in file specification: ${spec}")
  endif(len GREATER 2)
  if(len EQUAL 0)
    message(FATAL_ERROR "empty file specification")
  endif(len EQUAL 0)

  # use as-is
  if(len EQUAL 2)
    list(GET spec_items 0 instance)
    list(GET spec_items 1 template)
  endif(len EQUAL 2)

  # deduce
  if(len EQUAL 1)
    string(REGEX REPLACE ".in\\$" "" instance "${spec}")
    set(template "${instance}.in")
  endif(len EQUAL 1)

  #export
  if(NOT ("${template_var}" STREQUAL ""))
    set("${template_var}" "${template}" PARENT_SCOPE)
  endif(NOT ("${template_var}" STREQUAL ""))
  if(NOT ("${instance_var}" STREQUAL ""))
    set("${instance_var}" "${instance}" PARENT_SCOPE)
  endif(NOT ("${instance_var}" STREQUAL ""))
endfunction(dune_instance_parse_file_spec)

# build output file name: parse the file_spec into a template name and a base
# instance name.  Mangle the ID by replacing anything special with "_" and
# intersperse the result between basename and extension of the base instance
# name.  Use the result as the instance name.
function(dune_instance_from_id file_spec id template_var instance_var)
  dune_instance_parse_file_spec("${file_spec}" template base)

  # split into prefix and suffix
  if(base MATCHES "\\.")
    string(REGEX REPLACE "\\.[^.]*\$"      ""    prefix "${base}")
    string(REGEX REPLACE "^.*(\\.[^.]*)\$" "\\1" suffix "${base}")
  else(base MATCHES "\\.")
    set(prefix "${base}")
    set(suffix)
  endif(base MATCHES "\\.")

  # mangle the id
  string(REGEX REPLACE "[^a-zA-Z0-9]+" "_" mangled_id "${id}")
  string(REGEX REPLACE "^_+"           ""  mangled_id "${mangled_id}")
  string(REGEX REPLACE "_+\$"          ""  mangled_id "${mangled_id}")
  if(mangled_id STREQUAL "")
    message(FATAL_ERROR "\"${id}\" is empty after mangling")
  endif(mangled_id STREQUAL "")

  #export
  if(NOT ("${template_var}" STREQUAL ""))
    set("${template_var}" "${template}" PARENT_SCOPE)
  endif(NOT ("${template_var}" STREQUAL ""))
  if(NOT ("${instance_var}" STREQUAL ""))
    set("${instance_var}" "${prefix}_${mangled_id}${suffix}" PARENT_SCOPE)
  endif(NOT ("${instance_var}" STREQUAL ""))
endfunction(dune_instance_from_id)

# mimic the behaviour of configure_file(), placing relative paths in the
# current binary dir
function(dune_instance_apply_bindir fname_var)
  if(NOT (IS_ABSOLUTE ${fname_var}))
    set(${fname_var} "${CMAKE_CURRENT_BINARY_DIR}/${${fname_var}}" PARENT_SCOPE)
  endif()
endfunction(dune_instance_apply_bindir)


######################################################################
#
#  File generation
#

function(dune_instance_set_generated)
  # prepare instance substitution variables
  set(GENERATED_SOURCE
    "generated from ${TEMPLATE} by cmake -*- buffer-read-only:t -*- vim: set readonly:"
    PARENT_SCOPE)

  set(BINDIR_INSTANCE "${INSTANCE}")
  dune_instance_apply_bindir(BINDIR_INSTANCE)
  set(BINDIR_INSTANCE "${BINDIR_INSTANCE}" PARENT_SCOPE)
endfunction(dune_instance_set_generated)

# Read a template file and split it into three lists
# - content_parts contains the parts before, between, and after templates
# - template_parts contains the content of each template
# - template_names contains the names of each template
# The elements in the lists are quoted using dune_instance_quote_element() to
# protect against problems with empty elements and against cmakes list()
# command butchering it's own quoting.
function(dune_instance_parse_embedded name content_parts template_parts template_names)
  message(VERBOSE "Parsing ${name} for embedded templates")
  file(READ "${name}" content)
  # ensure that the file content ends in a newline, which makes searching for
  # template marker easier
  string(APPEND content "\n")

  set(content_list "")
  set(template_list "")
  set(template_name_list "")
  set(acc "")
  set(lineno 0)
  set(in_template FALSE)
  while(NOT (content STREQUAL ""))
    string(FIND "${content}" "\n" nextline)
    math(EXPR nextline "${nextline} + 1")

    string(SUBSTRING "${content}" 0 "${nextline}" line)
    string(SUBSTRING "${content}" "${nextline}" -1 content)
    math(EXPR lineno "${lineno} + 1")

    if(line MATCHES "(.*)(@((end)?template)([ \t]+([-+._/0-9a-zA-Z]*))?@)(.*)")
      set(prefix "${CMAKE_MATCH_1}")
      set(sep "${CMAKE_MATCH_2}")
      set(sep_keyword "${CMAKE_MATCH_3}")
      set(sep_name "${CMAKE_MATCH_6}")
      set(sep_suffix "${CMAKE_MATCH_7}")

      if(in_template)
        if(NOT (sep_keyword STREQUAL "endtemplate"))
          message(FATAL_ERROR "\
${name}:${lineno}: '${sep}' nested inside...
${name}:${template_lineno}: ...'${template_sep}' here")
        endif()

        if(NOT ((sep_name STREQUAL "") OR (sep_name STREQUAL template_name)))
          message(FATAL_ERROR "\
${name}:${template_lineno}: '${template_sep}' closed by nonmatching...
${name}:${lineno}: ...'${sep}' here")
        endif()

        dune_instance_quote_element(acc)
        list(APPEND template_list "${acc}")
        dune_instance_quote_element(template_name)
        list(APPEND template_name_list "${template_name}")

        set(in_template FALSE)
      else()
        if(NOT (sep_keyword STREQUAL "template"))
          message(FATAL_ERROR "${name}:${lineno}: Lone '${sep}'")
        endif()

        dune_instance_quote_element(acc)
        list(APPEND content_list "${acc}")
        set(template_sep "${sep}")
        set(template_name "${sep_name}")
        set(template_lineno "${lineno}")

        set(in_template TRUE)
      endif()
      set(acc "")
    else() # line did not match separator
      string(APPEND acc "${line}")
    endif()
  endwhile()

  if(in_template)
    message(FATAL_ERROR "${name}:${template_lineno}: Unclosed '${template_sep}'")
  endif()

  dune_instance_quote_element(acc)
  list(APPEND content_list "${acc}")

  set("${content_parts}" "${content_list}" PARENT_SCOPE)
  set("${template_parts}" "${template_list}" PARENT_SCOPE)
  set("${template_names}" "${template_name_list}" PARENT_SCOPE)
endfunction(dune_instance_parse_embedded)

# Take the name of a list variable containing content parts other than
# embedded templates and instantiate each part.  Put the result back into the
# same variable.  List elements are quoted.
function(dune_instance_generate_parts _parts_list)
  set(_acc "")
  foreach(_part IN LISTS "${_parts_list}")
    dune_instance_unquote_element(_part)
    string(CONFIGURE "${_part}" _part)
    dune_instance_quote_element(_part)
    list(APPEND _acc "${_part}")
  endforeach(_part)
  set("${_parts_list}" "${_acc}" PARENT_SCOPE)
endfunction(dune_instance_generate_parts)

function(dune_instance_generate_file TEMPLATE INSTANCE)
  if(("${INSTANCE}" STREQUAL "") OR ("${TEMPLATE}" STREQUAL ""))
    message(FATAL_ERROR "need both INSTANCE and TEMPLATE")
  endif(("${INSTANCE}" STREQUAL "") OR ("${TEMPLATE}" STREQUAL ""))

  # prepare instance substitution variables
  dune_instance_set_generated()

  # do the generation
  message(VERBOSE "Generating ${TEMPLATE} -> ${INSTANCE}")
  file(READ "${TEMPLATE}" _content)
  string(CONFIGURE "${_content}" _content)

  # make sure we did not generate this file before
  get_property(_seen SOURCE "${BINDIR_INSTANCE}" PROPERTY DUNE_INSTANCE_GENERATED)
  if(_seen)
    file(READ "${BINDIR_INSTANCE}" _oldcontent)
    if(NOT (_content STREQUAL _oldcontent))
      message(FATAL_ERROR "Attempt to generate ${INSTANCE} (from ${TEMPLATE}), "
        "which has already been generated with different content")
    endif()
    # otherwise, the content matches, so nothing to do
  else(_seen)
    # _seen was false, but the file may still be around from a previous cmake
    # run, only write if changed to avoid recompilations
    dune_write_changed_file("${BINDIR_INSTANCE}" "${_content}")
    set_property(SOURCE "${BINDIR_INSTANCE}" PROPERTY DUNE_INSTANCE_GENERATED TRUE)
    set_property(DIRECTORY APPEND
      PROPERTY CMAKE_CONFIGURE_DEPENDS "${TEMPLATE}")
  endif(_seen)
endfunction(dune_instance_generate_file)

# only write if the content changes, avoiding recompilations
function(dune_write_changed_file name content)
  if(EXISTS "${name}")
    file(READ "${name}" oldcontent)
    if(content STREQUAL oldcontent)
      return()
    endif()
  endif()
  file(WRITE "${name}" "${content}")
endfunction(dune_write_changed_file)

######################################################################
#
#  High-level interface commands
#

function(dune_instance_begin)
  cmake_parse_arguments(_arg
    "" # options
    "" # one_value_keywords
    "FILES" # multi_value_keywords
    ${ARGV}
    )
  if(DEFINED _arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED _arg_UNPARSED_ARGUMENTS)

  set(_all_content_parts "")
  set(_all_template_parts "")
  set(_all_template_names "")
  foreach(_spec IN LISTS _arg_FILES)
    dune_instance_parse_file_spec("${_spec}" TEMPLATE INSTANCE)
    dune_instance_set_generated()
    # reconfigure if the input template file changes
    set_property(DIRECTORY APPEND
      PROPERTY CMAKE_CONFIGURE_DEPENDS "${TEMPLATE}")
    dune_instance_parse_embedded("${TEMPLATE}"
      _file_content_parts _file_template_parts _file_template_names)
    dune_instance_generate_parts(_file_content_parts)

    dune_instance_quote_element(_file_content_parts)
    list(APPEND _all_content_parts "${_file_content_parts}")

    dune_instance_quote_element(_file_template_parts)
    list(APPEND _all_template_parts "${_file_template_parts}")

    dune_instance_quote_element(_file_template_names)
    list(APPEND _all_template_names "${_file_template_names}")
  endforeach(_spec)
  set(_DUNE_INSTANCE_GLOBAL_FILES "${_arg_FILES}" PARENT_SCOPE)
  set(_DUNE_INSTANCE_CONTENT_PARTS "${_all_content_parts}" PARENT_SCOPE)
  set(_DUNE_INSTANCE_TEMPLATE_PARTS "${_all_template_parts}" PARENT_SCOPE)
  set(_DUNE_INSTANCE_TEMPLATE_NAMES "${_all_template_names}" PARENT_SCOPE)

  set(DUNE_INSTANCE_GENERATED "" PARENT_SCOPE)
endfunction(dune_instance_begin)


function(dune_instance_add)
  cmake_parse_arguments(_arg
    "" # options
    "ID;TEMPLATE" # one_value_keywords
    "FILES" # multi_value_keywords
    ${ARGV}
    )

  if(DEFINED _arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED _arg_UNPARSED_ARGUMENTS)

  # ensure _arg_TEMPLATE is set, even if it is the empty value
  set(_arg_TEMPLATE "${_arg_TEMPLATE}")

  set(_template_used FALSE)

  ######################################################################
  # Instantiate global (list) templates
  set(_all_content_parts "")
  list(LENGTH _DUNE_INSTANCE_GLOBAL_FILES _file_count)
  foreach(_file_index RANGE "${_file_count}")
    # filter out the end of the range, this also works with empty ranges
    if(_file_index EQUAL _file_count)
      break()
    endif()
    list(GET _DUNE_INSTANCE_GLOBAL_FILES "${_file_index}" _spec)
    dune_instance_parse_file_spec("${_spec}" TEMPLATE INSTANCE)
    dune_instance_set_generated()

    list(GET _DUNE_INSTANCE_CONTENT_PARTS "${_file_index}" _content_parts)
    dune_instance_unquote_element(_content_parts)
    list(GET _DUNE_INSTANCE_TEMPLATE_PARTS "${_file_index}" _template_parts)
    dune_instance_unquote_element(_template_parts)
    list(GET _DUNE_INSTANCE_TEMPLATE_NAMES "${_file_index}" _template_names)
    dune_instance_unquote_element(_template_names)

    set(_content_parts_result "")
    list(LENGTH _template_parts _parts_count)
    foreach(_part_index RANGE "${_parts_count}")
      list(GET _content_parts "${_part_index}" _content_part)
      # The list of template parts should be one shorter than the list of
      # content parts
      if(_part_index LESS _parts_count)
        list(GET _template_names "${_part_index}" _template_name)
        dune_instance_unquote_element(_template_name)
        if(_template_name STREQUAL _arg_TEMPLATE)
          set(_template_used TRUE)

          list(GET _template_parts "${_part_index}" _template_part)
          dune_instance_unquote_element(_template_part)
          string(CONFIGURE "${_template_part}" _result)

          dune_instance_unquote_element(_content_part)
          string(APPEND _content_part "${_result}")
          dune_instance_quote_element(_content_part)
        endif()
      endif()
      list(APPEND _content_parts_result "${_content_part}")
    endforeach(_part_index)

    dune_instance_quote_element(_content_parts_result)
    list(APPEND _all_content_parts "${_content_parts_result}")
  endforeach(_file_index)
  set(_DUNE_INSTANCE_CONTENT_PARTS "${_all_content_parts}" PARENT_SCOPE)

  ######################################################################
  # instantiate per instance templates
  foreach(_spec IN LISTS _arg_FILES)
    set(_template_used TRUE)
    dune_instance_from_id("${_spec}" "${_arg_ID}" _template_file _instance_file)
    set(_bindir_instance_file "${_instance_file}")
    dune_instance_apply_bindir(_bindir_instance_file)
    dune_instance_generate_file("${_template_file}" "${_instance_file}")
    list(FIND DUNE_INSTANCE_GENERATED "${_bindir_instance_file}" _found_pos)
    if(_found_pos EQUAL -1)
      list(APPEND DUNE_INSTANCE_GENERATED "${_bindir_instance_file}")
    endif()
  endforeach(_spec)

  # if we did not instantiate anything, that is probably an error
  if(NOT _template_used)
    message(FATAL_ERROR "No embedded template matched template '${_arg_TEMPLATE}', and no instance template files were given")
  endif()
  set(DUNE_INSTANCE_GENERATED "${DUNE_INSTANCE_GENERATED}" PARENT_SCOPE)
endfunction(dune_instance_add)

function(dune_instance_end)
  if(ARGC GREATER 0)
    message(FATAL_ERROR "dune_instance_end() does not take any arguments")
  endif()

  ######################################################################
  # Write global instances
  list(LENGTH _DUNE_INSTANCE_GLOBAL_FILES _file_count)
  foreach(_file_index RANGE "${_file_count}")
    # filter out the end of the range, this also works with empty ranges
    if(_file_index EQUAL _file_count)
      break()
    endif()
    list(GET _DUNE_INSTANCE_GLOBAL_FILES "${_file_index}" _spec)
    dune_instance_parse_file_spec("${_spec}" TEMPLATE INSTANCE)
    set(BINDIR_INSTANCE "${INSTANCE}")
    dune_instance_apply_bindir(BINDIR_INSTANCE)

    # make sure we did not generate this file before
    get_property(_seen SOURCE "${BINDIR_INSTANCE}" PROPERTY DUNE_INSTANCE_GENERATED)
    if("${_seen}")
      message(FATAL_ERROR "Attempt to generate ${INSTANCE} (from ${TEMPLATE}), "
        "which has already been generated")
    endif("${_seen}")

    list(GET _DUNE_INSTANCE_CONTENT_PARTS "${_file_index}" _content_parts)
    dune_instance_unquote_element(_content_parts)

    set(_content "")
    foreach(_part IN LISTS _content_parts)
      dune_instance_unquote_element(_part)
      string(APPEND _content "${_part}")
    endforeach(_part)
    # remove the final newline that we appended when reading the template file
    string(REGEX REPLACE "\n\$" "" _content "${_content}")

    message(VERBOSE "Writing ${INSTANCE}")
    # only write if the content changes, avoiding recompilations
    dune_write_changed_file("${BINDIR_INSTANCE}" "${_content}")

    set_property(SOURCE "${BINDIR_INSTANCE}" PROPERTY DUNE_INSTANCE_GENERATED TRUE)
    list(APPEND DUNE_INSTANCE_GENERATED "${BINDIR_INSTANCE}")
  endforeach(_file_index)

  set(DUNE_INSTANCE_GENERATED "${DUNE_INSTANCE_GENERATED}" PARENT_SCOPE)
endfunction(dune_instance_end)
