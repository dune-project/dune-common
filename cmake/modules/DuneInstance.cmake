# Module to generate instantiations, typically for some template
#
# .. cmake_module::
#
#    This module can be used to generate explicit template instantiations.
#    Suppose you have a template test function that you want to call for a
#    number of template arguments.  You want to explicitly instantiate the
#    function for each set of template arguments, and put the instanciation
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
#    instances of ``MyTestSuite::test()`` are instantiated explitly so they do
#    not instantiate them implicitly themselves (that would violate the
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
#    :ref:`dune_instance_template() <dune_instance_template>`,
#    :ref:`dune_instance_add() <dune_instance_add>`, and
#    :ref:`dune_instance_end() <dune_instance_end>`, which can be used to
#    automatically generate the explicit instantiations for each type and the
#    contents for the header and the body of main.
#
#    This may loo like this in ``CMakeLists.txt``::
#
#      dune_instance_begin(COLLECTORS BODY HEADER
#                          OUTPUTS MYTEST_SOURCES)
#      dune_instance_template(
#        BODY "suite.test<@TYPE@>();\n"
#        HEADER "extern template void MyTestSuite::test<@TYPE@>();\n"
#        FILES mytest_instance.cc)
#
#      foreach(TYPE IN ITEMS bool char int double)
#        dune_instance_add(ID "${TYPE}")
#      endforeach(TYPE IN ITEMS bool char int double)
#
#      dune_instance_end(FILES mytest.cc mytest.hh)
#
#      dune_add_test(NAME mytest
#        SOURCES mytest.cc ${MYTEST_SOURCES})
#
#    The call to :ref:`dune_instance_begin() <dune_instance_begin>` declares
#    that each instance will append something to the variables ``BODY`` and
#    ``HEADER`` -- we will collect the contents of the header and the body of
#    main there.  The list of generated instance files will be kept in the
#    variable ``MYTEST_SOURCES``.
#
#    :ref:`dune_instance_template() <dune_instance_template>` defines a
#    template.  It defines template strings for the body and header content.
#    It also defines a base filename to use for the explicit template
#    definitions.  This command must be invoked after
#    :ref:`dune_instance_begin() <dune_instance_begin>` since it needs to know
#    which the collector variables.
#
#    The call to :ref:`dune_instance_add() <dune_instance_add>` occurs in a
#    loop.  Each call will append to the variables ``HEADER`` and ``BODY``,
#    using the template strings defined earlier.  These template strings are
#    run through the cmake command ``string(CONFIGURE ...)`` to replace
#    occurance of ``@TYPE@`` by the value of the variable ``TYPE`` set in the
#    for loop.  Then files containing explicit instantiatons will be generated
#    as ``mytest_instance_bool.cc``, ``mytest_instance_bool.cc``, etc, from a
#    template file ``mytest_instance.cc.in``.  The name of the generated files
#    are the base file name from the template definition with the ``ID``
#    inserted before the extension.  The name of the template file is the same
#    base file name with ``.in`` appended.
#
#    :ref:`dune_instance_end() <dune_instance_end>` is used to generate
#    ``mytest.cc`` and ``mytest.hh`` from ``mytest.cc.in`` and
#    ``mytest.cc.hh``.  These contain ``@HEADER@`` and ``@BODY@``
#    substitutions to insert the respective collected content.  These files
#    are currently not included in the list of generated files
#    ``MYTEST_SOURCES`` since the ``.hh`` file should not be included in the
#    list of sources when defining a target.
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
#      @BODY@
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
#      @HEADER@
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
#    human reader that this wile was generated and what the template file was,
#    and it hints editors to go into read-only mode.
#
#    .. rubric:: How Files And Strings Are Generated
#
#    The generation is done using the cmake command ``configure_file(...)``
#    for template files and ``string(CONFIGURE ...)`` for template strings.
#    These simply substitute the current variable values, so make sure to set
#    up the variables to substitute before calling :ref:`dune_instance_add()
#    <dune_instance_add>` or :ref:`dune_instance_end() <dune_instance_end>`.
#
#    Refrain from using substitutions that begin with an underscore
#    (i.e. ``@_my_local_var@``).  The generation functions in this module use
#    such names for their local variables and may hide the variable you are
#    trying to substitude.
#
#    When instantiating files we set up a few convenience variables before
#    calling ``configure_file()`` that can be used in substitutions:
#    ``@TEMPLATE@`` contains the name of the template file, ``@INSTANCE@``
#    contains the name of the file being generated.  ``@GENERATED_SOURCE@``
#    contains a one-line message that this file was generated, including the
#    name of the template file.
#
#    .. rubric:: Main Interface Commands
#
#    These are the ones you normally use.
#
#    * :ref:`dune_instance_begin() <dune_instance_begin>`
#    * :ref:`dune_instance_template() <dune_instance_template>`
#    * :ref:`dune_instance_add() <dune_instance_add>`
#    * :ref:`dune_instance_end() <dune_instance_end>`
#
#    .. rubric:: Utility Commands
#
#    You would not use these directly under normal circumstances.
#
#    * :ref:`dune_instance_parse_file_spec() <dune_instance_parse_file_spec>`
#    * :ref:`dune_instance_from_id() <dune_instance_from_id>`
#    * :ref:`dune_instance_generate_file() <dune_instance_generate_file>`
#    * :ref:`dune_instance_generate_files() <dune_instance_generate_files>`
#
#
# .. cmake_function:: dune_instance_begin
#
#    .. cmake_brief::
#
#       Prepare for a list of instances.
#
#    .. cmake_param:: COLLECTORS
#       :multi:
#       :argname: collector_var
#
#       Variables in the scope of the caller to collect content generated from
#       string templates in.
#
#    .. cmake_param:: OUTPUTS
#       :single:
#       :argname: outputs_var
#
#       Name of a cmake variable in the scope of the caller to store the list
#       of generated instance files in.
#
#    Clear the internal variables that hold the accumulated lists content, as
#    well as the list of generated files.
#
#    .. note::
#
#       There should normally be a matching :ref:`dune_instance_end()
#       <dune_instance_end>`, although it is not strictly required.  Since
#       information is communicated through variables in the callers scope,
#       :ref:`dune_instance_begin()
#       <dune_instance_begin>`/:ref:`dune_instance_end() <dune_instance_end>`
#       blocks may not be nested inside the same scope.  Since a function is a
#       new scope, it may safely contain a :ref:`dune_instance_begin()
#       <dune_instance_begin>`/:ref:`dune_instance_end() <dune_instance_end>`
#       block, even if it is itself called from one.
#
#
# .. cmake_function:: dune_instance_template
#
#    .. cmake_brief::
#
#       Define a template for later use.
#
#    .. cmake_param:: NAME
#       :single:
#
#       Name of the template, defaults to ``DEFAULT``.
#
#    .. cmake_param:: *COLLECTORS*
#       :special:
#       :argname: *COLLECTOR1* template_string1 [*COLLECTOR2* template_string2 ...]
#
#       Each pair of ``COLLECTOR template_string`` defines a template string
#       for the given collector.
#
#       There may be multiple instances of this parameter, one for each
#       collector defined in :ref:`dune_instance_begin()
#       <dune_instance_begin>`.  If a collector isn't given it's
#       `string_template` defaults to empty.
#
#    .. cmake_param:: FILES
#       :multi:
#
#       List of file specifications to generate for each instance.  These are
#       usually the names of template files with the ``.in`` extension
#       removed.  See :ref:`dune_instance_add() <dune_instance_add>` for
#       details.
#
#    Define a template for use in :ref:`dune_instance_add()
#    <dune_instance_add>`.  The template is stored in cmake variables in the
#    scope of the caller.  Calls to ``dune_instance_template()`` must appear
#    after the corresponding :ref:`dune_instance_begin()
#    <dune_instance_begin>` so the list of collectors in known.
#
#    It is permissible to redefine an existing template.
#
#
# .. cmake_function:: dune_instance_add
#
#    .. cmake_brief::
#
#       Instantiate a template with the currently set variable values.
#
#    .. cmake_param:: ID
#       :single:
#       :required:
#
#       Used to build the names of generated files.  Each file specification
#       given in the template together with this id is given to
#       :ref:`dune_instance_from_id() <dune_instance_from_id>` to determine
#       the name of a template file and the name of an instance file.  To get
#       unique instance file names this ID should usually a list of variable
#       values joined together by ``_``.
#
#       Specifically, each file specification may be of the form
#       ``template_file_name:base_instance_file_name``, or it may be a single
#       token not containing ``:``.  In the latter case, if that token
#       contains a trailing ``.in``, that is remove and the result it the base
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
#       Set the name of the template to use.  Defaults to ``DEFAULT``.
#
#    Append content to the collector variables generated from the template
#    strings in the given template, substituting the current variables values.
#    Then, generate files from the according the the file specifications in
#    the template, doing substitutions as well.
#
#
# .. cmake_function:: dune_instance_end
#
#    .. cmake_brief::
#
#       Close a block started by :ref:`dune_instance_begin()
#       <dune_instance_begin>`, generate files with collector content.
#
#    .. cmake_param:: FILES
#       :multi:
#       :argname: file_spec
#
#       Each file specification can be the name of a template file if it has
#       ``.in`` at the end, or the name of an instance file if it doesn't.
#       The name of the other file is obtained by appending or removing
#       ``.in``, as applicable.  Both file names can also be given explicitly
#       in the form ``template_file_name:instance_file_name``.
#
#    For each file specification, generate files using
#    :ref:`dune_instance_generate_files() <dune_instance_generate_files>`.
#    This is typically used to substitute the content of collector variables.
#
#    .. note::
#
#       ``dune_instance_end()`` is mostly syntactic sugar.  You could directly
#       use :ref:`dune_instance_generate_files()
#       <dune_instance_generate_files>` instead.  ``dune_instance_end()`` does
#       not clear any variables, so you can continue to use their contents
#       afterwards.
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
#       :ref:`dune_instance_end() <dune_instance_end>` and
#       :ref:`dune_instance_generate_files() <dune_instance_generate_files>`.
#       It is also used as a helper in :ref:`dune_instance_from_id()
#       <dune_instance_from_id>` to determine template file name and base
#       instance file name.
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
#       The id specification.  This should be uniquely identify an instance.
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
#       :ref:`dune_instance_template(FILES ...) <dune_instance_template>` when
#       :ref:`dune_instance_add() <dune_instance_add>` is invoked.
#
#
# .. cmake_function:: dune_instance_generate_file
#
#    .. cmake_brief::
#
#       Convenience wrapper around ``configure_file()``: enable standard
#       substitutions and register files as generated.
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
#       The name of the generated file.
#
#    Make sure the variables ``TEMPLATE`` and ``INSTANCE`` are set to the
#    parameter values and available for substitution.  Also set the variable
#    ``GENERATED_SOURCE`` to a one-line message that tells a human reader that
#    this file is generated, and the name of the template file it was
#    generated from.  The message also includes hints for common editors
#    telling them to switch to read-only mode.
#
#    Then call ``configure_file()`` and register the instance file a a
#    generated source file.
#
#    If the instance file has been registered as a generated source file
#    before, this function generates a fatal error.  This ensures that any
#    accidential attempt to generate the same file twice is caught.
#
#
# .. cmake_function:: dune_instance_generate_files
#
#    .. cmake_brief::
#
#       Generate multiple files from file specifications.
#
#    .. cmake_param:: file_specs
#       :special:
#       :argname: file_spec1 [file_spec2 ...]
#
#       File specifications.
#
#    Parse each file specification using :ref:`dune_instance_parse_file_spec()
#    <dune_instance_parse_file_spec>`, and call
#    :ref:`dune_instance_generate_file() <dune_instance_generate_file>` with
#    the result.
#
#    .. note::
#
#       This exists mainly to give you an opportunity to generate files with
#       collector content manually in a particular order in the same manner as
#       other files generated using this module.  Normally is should simply be
#       invoked through :ref:`dune_instance_end() <dune_instance_end>`.


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

######################################################################
#
#  File generation
#

function(dune_instance_generate_file TEMPLATE INSTANCE)
  if(("${INSTANCE}" STREQUAL "") OR ("${TEMPLATE}" STREQUAL ""))
    message(FATAL_ERROR "need both INSTANCE and TEMPLATE")
  endif(("${INSTANCE}" STREQUAL "") OR ("${TEMPLATE}" STREQUAL ""))

  # make sure we did not generate this file before
  get_property(_seen SOURCE "${INSTANCE}" PROPERTY GENERATED)
  if("${_seen}")
    message(FATAL_ERROR "Attempt to generate ${INSTANCE} (from ${TEMPLATE}), "
      "which has already been generated")
  endif("${_seen}")

  # prepare instance substitution variables
  set(GENERATED_SOURCE "generated from ${TEMPLATE} by cmake   "
    "-*- buffer-read-only:t -*- vim: set readonly:")

  # do the generation
  message(STATUS "Generating ${TEMPLATE} -> ${INSTANCE}")
  configure_file("${TEMPLATE}" "${INSTANCE}")
  set_property(SOURCE "${INSTANCE}" PROPERTY GENERATED TRUE)
endfunction(dune_instance_generate_file)

function(dune_instance_generate_files)
  foreach(_file IN LISTS ARGV)
    dune_instance_parse_file_spec("${_file}" _template _instance)
    dune_instance_generate_file("${_template}" "${_instance}")
  endforeach(_file IN LISTS ARGV)
endfunction(dune_instance_generate_files)


######################################################################
#
#  High-level interface commands
#

function(dune_instance_begin)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "" # options
    "OUTPUTS" # one_value_keywords
    "COLLECTORS" # multi_value_keywords
    )
  if(DEFINED arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED arg_UNPARSED_ARGUMENTS)

  set(DUNE_INSTANCE_COLLECTORS "${arg_COLLECTORS}" PARENT_SCOPE)
  foreach(var IN LISTS arg_COLLECTORS)
    set("${var}" "" PARENT_SCOPE)
  endforeach(var IN LISTS arg_COLLECTORS)

  set(DUNE_INSTANCE_OUTPUTS "${arg_OUTPUTS}" PARENT_SCOPE)
  set("${arg_OUTPUTS}" "" PARENT_SCOPE)
endfunction(dune_instance_begin)


function(dune_instance_template)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "" # options
    "NAME;${DUNE_INSTANCE_COLLECTORS}" # one_value_keywords
    "FILES" # multi_value_keywords
    )

  if(DEFINED arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED arg_UNPARSED_ARGUMENTS)

  if(NOT DEFINED arg_NAME)
    set(arg_NAME DEFAULT)
  endif(NOT DEFINED arg_NAME)

  set("DUNE_INSTANCE_TEMPLATE_${arg_NAME}_FILES" "${arg_FILES}" PARENT_SCOPE)
  foreach(c IN LISTS DUNE_INSTANCE_COLLECTORS)
    set("DUNE_INSTANCE_TEMPLATE_${arg_NAME}_${c}" "${arg_${c}}" PARENT_SCOPE)
  endforeach(c IN LISTS DUNE_INSTANCE_COLLECTORS)
endfunction(dune_instance_template)


function(dune_instance_add)
  cmake_parse_arguments(PARSE_ARGV 0 _arg
    "" # options
    "ID;TEMPLATE" # one_value_keywords
    "" # multi_value_keywords
    )

  if(DEFINED _arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED _arg_UNPARSED_ARGUMENTS)

  # determine template to use
  if("${_arg_TEMPLATE}" STREQUAL "")
    set(_arg_TEMPLATE DEFAULT)
  endif("${_arg_TEMPLATE}" STREQUAL "")

  # append collectors
  foreach(_collector IN LISTS DUNE_INSTANCE_COLLECTORS)
    string(CONFIGURE
      "${DUNE_INSTANCE_TEMPLATE_${_arg_TEMPLATE}_${_collector}}" _content)
    string(APPEND "${_collector}" "${_content}")
    set("${_collector}" "${${_collector}}" PARENT_SCOPE)
  endforeach(_collector IN LISTS DUNE_INSTANCE_COLLECTORS)

  # generate instance files
  foreach(_spec IN LISTS "DUNE_INSTANCE_TEMPLATE_${_arg_TEMPLATE}_FILES")
    # determine instance and template file name
    dune_instance_from_id("${_spec}" "${_arg_ID}" TEMPLATE INSTANCE)

    # generate instanciation
    dune_instance_generate_file("${TEMPLATE}" "${INSTANCE}")

    # remember generated file
    list(APPEND "${DUNE_INSTANCE_OUTPUTS}" "${INSTANCE}")
  endforeach(_spec IN LISTS "DUNE_INSTANCE_TEMPLATE_${_arg_TEMPLATE}_FILES")

  # export generated files
  set("${DUNE_INSTANCE_OUTPUTS}" "${${DUNE_INSTANCE_OUTPUTS}}" PARENT_SCOPE)
endfunction(dune_instance_add)

function(dune_instance_end)
  cmake_parse_arguments(PARSE_ARGV 0 _arg
    "" # options
    "" # one_value_keywords
    "FILES" # multi_value_keywords
    )

  if(DEFINED _arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED _arg_UNPARSED_ARGUMENTS)

  dune_instance_generate_files(${_arg_FILES})
endfunction(dune_instance_end)
