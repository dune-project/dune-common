# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Module to generate instantiations, typically for some template
#
# .. cmake_module::
#
#    This module can be used to generate explicit template instantiations.
#    Suppose you have a template test function that you want to call for a
#    number of template arguments.  You want to explicitly instantiate the
#    function for each set of template arguments, and put the instantiation
#    into its own translation unit.  (This can be beneficial in that it limits
#    the amount of code that the optimizer sees at once, and thus it can
#    reduce both memory and cpu requirements during compilation.)
#
#    .. rubric:: Examples
#
#    Let's say you are writing a test ``mytest.cc`` and need to call a
#    template function for several types::
#
#      #include <mytestsuite.hh>
#
#      int main() {
#        MyTestSuite suite;
#
#        suite.test<bool>();
#        suite.test<char>();
#        suite.test<int>();
#        suite.test<double>();
#
#        return suite.good() ? EXIT_SUCCESS : EXIT_FAILURE;
#      }
#
#    Let's further say that you want to explicitly instantiate each used
#    ``MyTestSuite::test()`` instance in it's own translation unit.  Then you
#    need a series of files ``mytest_instance_bool.cc``,
#    ``mytest_instance_char.cc``, etc, all with essentially the same content::
#
#      #include <mytestsuite.hh>
#
#      template void MyTestSuite::test<@TYPE@>();
#
#    where ``@TYPE@`` is replaced by ``bool``, ``char``, etc as appropriate.
#
#    This is however not enough: all translation units need to know which
#    instances of ``MyTestSuite::test()`` are instantiated explicitly so they
#    do not instantiate them implicitly themselves (that would violate the
#    one-definition rule).  C++ only allows to declare individual instances as
#    extern, not all of them collectively, so we need to put a list of all
#    these instances into a header ``mytest.hh``::
#
#      #include <mytestsuite.hh>
#
#      extern template void MyTestSuite::test<bool>();
#      extern template void MyTestSuite::test<char>();
#      extern template void MyTestSuite::test<int>();
#      extern template void MyTestSuite::test<double>();
#
#    We also need to include that header from each translation unit in the
#    test, we can simply replace ``#include <mytestsuite.hh>`` with ``#include
#    <mytest.hh>``.
#
#    This is of course tedious and prone tp break if the list of tested types
#    changes.  To make this less fragile this module provides a series of
#    commands: :ref:`dune_instance_begin() <dune_instance_begin>`,
#    :ref:`dune_instance_add() <dune_instance_add>`, and
#    :ref:`dune_instance_end() <dune_instance_end>`, which can be used to
#    automatically generate the explicit instantiations for each type and the
#    contents for the header and the body of main.
#
#    This may look like this in ``CMakeLists.txt``::
#
#      dune_instance_begin(FILES mytest.cc mytest.hh)
#
#      foreach(TYPE IN ITEMS bool char int double)
#        dune_instance_add(ID "${TYPE}" FILES mytest_instance.cc)
#      endforeach(TYPE IN ITEMS bool char int double)
#
#      dune_instance_end()
#
#      list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
#      dune_add_test(NAME mytest
#        SOURCES ${DUNE_INSTANCE_GENERATED})
#
#    The call to :ref:`dune_instance_begin() <dune_instance_begin>` reads
#    ``mytest.cc.in`` and ``mytest.hh.in`` and splits them into embedded
#    templates and other content.  It will replace occurrences of ``@VAR@``
#    now in the other content and save the result for later.
#
#    The call to :ref:`dune_instance_add() <dune_instance_add>` occurs in a
#    loop.  Each call will instantiate the embedded templates extracted
#    earlier to replace an occurrence of ``@TYPE@`` by the value of the variable
#    ``TYPE`` set in the for loop.  Then files containing explicit
#    instantiatons will be generated as ``mytest_instance_bool.cc``,
#    ``mytest_instance_bool.cc``, etc, from a template file
#    ``mytest_instance.cc.in``.  The name of the generated files are the base
#    file name from the template definition with the ``ID`` inserted before
#    the extension.  The name of the template file is the same base file name
#    with ``.in`` appended.
#
#    :ref:`dune_instance_end() <dune_instance_end>` is used to write
#    ``mytest.cc`` and ``mytest.hh`` with the collected content from the
#    embedded templates.  The list of generated files will be available in the
#    variable ``DUNE_INSTANCE_GENERATED``.
#
#    The template files then look like this:
#
#    ``mytest.cc.in``::
#
#      // @GENERATED_SOURCE@
#
#      #include <config.h>
#
#      #include <mytest.hh>
#
#      int main() {
#        MyTestSuite suite;
#
#      #cmake @template@
#        suite.test<@TYPE@>();
#      #cmake @endtemplate@
#
#        return suite.good() ? EXIT_SUCCESS : EXIT_FAILURE;
#      }
#
#    ``mytest.hh.in``::
#
#      // @GENERATED_SOURCE@
#
#      #include <mytestsuite.hh>
#
#      #cmake @template@
#      extern template void MyTestSuite::test<@TYPE@>();
#      #cmake @endtemplate@
#
#    ``mytest_instance.cc.in``::
#
#      // @GENERATED_SOURCE@
#
#      #include <config.h>
#
#      #include <mytest.hh>
#
#      template void MyTestSuite::test<@TYPE@>();
#
#    The ``@GENERATED_SOURCE@`` substitution is good practice, it tells a
#    human reader that this file was generated and what the template file was,
#    and it hints editors to go into read-only mode.
#
#    .. rubric:: Embedded Templates
#
#    The template files given in :ref:`dune_instance_begin()
#    <dune_instance_begin>` can contain embedded templates.  These will be
#    instantiated by :ref:`dune_instance_add() <dune_instance_add>`, and all
#    instantiations will be concatenated together and replace the original
#    embedded template.
#
#    The begin of an embedded template is marked by a line containing
#    ``@template@`` or ``@template NAME@``.  Leaving off the name is
#    equivalent to an empty name.  ``dune_instance_add(TEMPLATE NAME)`` will
#    only instantiate embedded templates whose name matches and ignore all
#    others.
#
#    The end of an embedded template is marked by a line containing
#    ``@endtemplate@`` or ``@endtemplate NAME@``.  If a name is given, it must
#    match the name of the embedded template it closes.  If no name is given
#    (or the name is empty), that check is omitted.
#
#    There may be arbitrary characters on the same line before or after the
#    begin and end markers.  These are ignored, so you can use them for
#    comments or to trick your editor into proper indentation.  The one
#    exception is that the line surrounding the marker may not contain any
#    ``@`` characters to avoid ambiguities.
#
#    .. rubric:: How Files And Strings Are Generated
#
#    The generation is done using the cmake command ``configure_file(...)``
#    for template files and ``string(CONFIGURE ...)`` for template strings.
#    These simply substitute the current variable values, so make sure to set
#    up the variables to substitute before calling :ref:`dune_instance_add()
#    <dune_instance_add>` or :ref:`dune_instance_begin()
#    <dune_instance_begin>`.
#
#    Refrain from using substitutions that begin with an underscore
#    (i.e. ``@_my_local_var@``).  The generation functions in this module use
#    such names for their local variables and may hide the variable you are
#    trying to substitute.
#
#    When instantiating files we set up a few convenience variables before
#    calling ``configure_file()`` that can be used in substitutions:
#    ``@TEMPLATE@`` contains the name of the template file.  ``@INSTANCE@``
#    contains the name of the file being generated, not including an implied
#    ``${CMAKE_CURRENT_BINARY_DIR}``.  Use ``@BINDIR_INSTANCE@`` if you do
#    want the implied ``${CMAKE_CURRENT_BINARY_DIR}``.  ``@GENERATED_SOURCE@``
#    contains a one-line message that this file was generated, including the
#    name of the template file.
#
#    .. rubric:: Main Interface
#
#    These are the ones you normally use.
#
#    * :ref:`dune_instance_begin() <dune_instance_begin>`
#    * :ref:`dune_instance_add() <dune_instance_add>`
#    * :ref:`dune_instance_end() <dune_instance_end>`
#    * :ref:`DUNE_INSTANCE_GENERATED <DUNE_INSTANCE_GENERATED>`
#
#    .. rubric:: Utilities
#
#    You would not use these directly under normal circumstances.
#
#    * :ref:`dune_instance_parse_file_spec() <dune_instance_parse_file_spec>`
#    * :ref:`dune_instance_from_id() <dune_instance_from_id>`
#    * :ref:`dune_instance_generate_file() <dune_instance_generate_file>`
#
#
# .. cmake_function:: dune_instance_begin
#
#    .. cmake_brief::
#
#       Prepare for a list of instances.
#
#    .. cmake_param:: FILES
#       :multi:
#       :argname: file_spec
#
#       List of template files with embedded templates.
#
#    Read the given template files, and extract embedded templates.  Run the
#    generator on the remaining file content with the variables currently in
#    effect.
#
#    .. note::
#
#       A matching :ref:`dune_instance_end() <dune_instance_end>` is required.
#       Since information is communicated through variables in the callers
#       scope, :ref:`dune_instance_begin()
#       <dune_instance_begin>`/:ref:`dune_instance_end() <dune_instance_end>`
#       blocks may not be nested inside the same scope.  Since a function is a
#       new scope, it may safely contain a :ref:`dune_instance_begin()
#       <dune_instance_begin>`/:ref:`dune_instance_end() <dune_instance_end>`
#       block, even if it is itself called from one.
#
#
# .. cmake_function:: dune_instance_add
#
#    .. cmake_brief::
#
#       Instantiate a template with the currently set variable values.
#
#    .. cmake_param:: FILES
#       :multi:
#       :argname: file_spec
#
#       List of template file specifications.  These are usually the names of
#       template files with the ``.in`` extension removed.  See the ID
#       parameter for details.
#
#    .. cmake_param:: ID
#       :single:
#
#       Used to build the names of generated files.  Each file specification
#       together with this id is given to :ref:`dune_instance_from_id()
#       <dune_instance_from_id>` to determine the name of a template file and
#       the name of an instance file.  To get unique instance file names this
#       ID should usually be a list of variable values joined together by
#       ``_``.
#
#       Specifically, each file specification may be of the form
#       ``template_file_name:base_instance_file_name``, or it may be a single
#       token not containing ``:``.  In the latter case, if that token
#       contains a trailing ``.in``, that is removed and the result is the base
#       instance file name.  The base instance file name has the ``.in``
#       appended again to form the template file name.
#
#       The template file name is used as-is to generate files from.
#
#       The ID is mangled by replacing any runs of non-alphanumeric characters
#       with an underscore ``_``, and stripping any resulting underscore from
#       the beginning and the end.  The result is inserted before any
#       extension into the base instance file name to form the instance file
#       name.
#
#    .. cmake_param:: TEMPLATE
#       :single:
#
#       Instantiate embedded templates by this name.  Defaults to an empty
#       name, matching embedded templates without name.
#
#    Instantiate any embedded templates that match the given template name,
#    substituting the current variables values.  Then, generate files
#    according the the file specifications in the template, doing
#    substitutions as well.
#
#
# .. cmake_function:: dune_instance_end
#
#    .. cmake_brief::
#
#       Close a block started by :ref:`dune_instance_begin()
#       <dune_instance_begin>`, and write the files generated from the
#       templates given there.
#
#    Write the files generated from the template files given in
#    :ref:`dune_instance_begin() <dune_instance_begin>`, including any content
#    generated from embedded templates in :ref:`dune_instance_add()
#    <dune_instance_add>`.
#
#
# .. cmake_function:: dune_instance_parse_file_spec
#
#    .. cmake_brief::
#
#       Parse a file specification into a template file name and an instance
#       file name.
#
#    .. cmake_param:: spec
#       :positional:
#       :single:
#       :required:
#
#       The file specification.
#
#    .. cmake_param:: template_var
#       :positional:
#       :single:
#
#       Name of the variable to store the template file name in.  Can be empty
#       to discard the template file name.
#
#    .. cmake_param:: instance_var
#       :positional:
#       :single:
#
#       Name of the variable to store the instance file name in.  Can be empty
#       to discard then instance file name.
#
#    The file specification can be the name of a template file if it has
#    ``.in`` at the end, or the name of an instance file if it doesn't.  The
#    name of the other file is obtained by appending or removing ``.in``, as
#    applicable.  Both file names can also be given explicitly in the form
#    ``template_file_name:instance_file_name``.
#
#    .. note::
#
#       This is the function use to parse the file specifications in
#       :ref:`dune_instance_begin() <dune_instance_begin>`.  It is also used
#       as a helper in :ref:`dune_instance_from_id() <dune_instance_from_id>`
#       to determine template file name and base instance file name.
#
#
# .. cmake_function:: dune_instance_from_id
#
#    .. cmake_brief::
#
#       Determine a template file name and an instance file name from a file
#       specification and a unique id.
#
#    .. cmake_param:: file_spec
#       :positional:
#       :single:
#       :required:
#
#       The file specification.
#
#    .. cmake_param:: id
#       :positional:
#       :single:
#       :required:
#
#       The id specification.  This should uniquely identify an instance.
#
#    .. cmake_param:: template_var
#       :positional:
#       :single:
#
#       Name of the variable to store the template file name in.  Can be empty
#       to discard the template file name.
#
#    .. cmake_param:: instance_var
#       :positional:
#       :single:
#
#       Name of the variable to store the instance file name in.  Can be empty
#       to discard the instance file name.
#
#    The file specification is handed to :ref:`dune_instance_parse_file_spec()
#    <dune_instance_parse_file_spec>` to determine a template file name and a
#    *base* instance file name.
#
#    The ID is mangled by replacing any runs of non-alphanumeric characters
#    with an underscore ``_``, and stripping any resulting underscore from the
#    beginning and the end.  The result is inserted before any extension into
#    the base instance file name to form the instance file name.
#
#    .. note::
#
#       This is the function use to parse the file specifications given in
#       :ref:`dune_instance_add(FILES ...) <dune_instance_add>`.
#
#
# .. cmake_function:: dune_instance_apply_bindir
#
#    .. cmake_brief::
#
#       Modify a filename to be relative to ``CMAKE_CURRENT_BINARY_DIR``.
#
#    .. cmake_param:: fname_var
#       :positional:
#       :single:
#       :required:
#
#       The name of the variable containing the file name.
#
#    This is used to mimic the behaviour of ``configure_file()``.  If the file
#    name given is not absolute, it is modified by prepending
#    ``${CMAKE_CURRENT_BINARY_DIR}``.
#
#
# .. cmake_function:: dune_instance_generate_file
#
#    .. cmake_brief::
#
#       Convenience replacement for ``configure_file()``: enable standard
#       substitutions, register files as generated, and flag the same file
#       being generated twice.
#
#    .. cmake_param:: TEMPLATE
#       :positional:
#       :single:
#       :required:
#
#       The name of the template file.
#
#    .. cmake_param:: INSTANCE
#       :positional:
#       :single:
#       :required:
#
#       The name of the generated file.  This is assumed relative to
#       ``${CMAKE_CURRENT_BINARY_DIR}``.
#
#    Make sure the variables ``TEMPLATE``, ``INSTANCE``, and
#    ``BINDIR_INSTANCE`` are set to the parameter values and available for
#    substitution.  Also set the variable ``GENERATED_SOURCE`` to a one-line
#    message that tells a human reader that this file is generated, and the
#    name of the template file it was generated from.  The message also
#    includes hints for common editors telling them to switch to read-only
#    mode.
#
#    Then generate the file as if by ``configure_file()``.
#
#    If the instance file has been registered as a generated source file
#    before, this function generates a fatal error.  This ensures that any
#    accidental attempt to generate the same file twice is caught.  As a
#    special exception, if the generated content is the same as before, the
#    error is silently skipped.
#
#
# .. cmake_variable:: DUNE_INSTANCE_GENERATED
#
#    After :ref:`dune_instance_end() <dune_instance_end>`, this holds the list
#    of files that were generated.  The list entries include an implied
#    ``${CMAKE_CURRENT_BINARY_DIR}``, as appropriate.
#
#    Do not rely on the value of this variable and do not modify it inside a
#    :ref:`dune_instance_begin()
#    <dune_instance_begin>`/:ref:`dune_instance_end() <dune_instance_end>`
#    block.
include_guard(GLOBAL)

# macro to print additional information to the cmake output file.
# Note: in cmake 3.15 this is available through the message(VERBOSE "...") function.
macro(message_verbose TEXT)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    message(VERBOSE "${TEXT}")
  else()
    file(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log "${TEXT}\n")
  endif()
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
  message_verbose("Parsing ${name} for embedded templates")
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
  message_verbose("Generating ${TEMPLATE} -> ${INSTANCE}")
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

    message_verbose("Writing ${INSTANCE}")
    # only write if the content changes, avoiding recompilations
    dune_write_changed_file("${BINDIR_INSTANCE}" "${_content}")

    set_property(SOURCE "${BINDIR_INSTANCE}" PROPERTY DUNE_INSTANCE_GENERATED TRUE)
    list(APPEND DUNE_INSTANCE_GENERATED "${BINDIR_INSTANCE}")
  endforeach(_file_index)

  set(DUNE_INSTANCE_GENERATED "${DUNE_INSTANCE_GENERATED}" PARENT_SCOPE)
endfunction(dune_instance_end)
