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
#    For each instanciation you need to do three things:
#
#    1. Write code in the body of ``main()`` (or a similar place) that calls
#       the instance,
#
#    2. write an ``extern`` declaration for the instance in some header that
#       is included by the main program as well as each translation unit of
#       the instantiations, and
#
#    3. write a ``.cc`` file that explicitly instantiates the template
#       function.
#
#    The header with the ``extern`` declarations needs to be included by the
#    instatiation ``.cc`` files too so all translation units have a coherent
#    view of which instance of the template is explicitly instantiated.  This
#    is particularly important if on instance may call another instance.
#
#    For each of the things above, you need to provide a template::
#
#      define_template(NAME mytemplate
#          BODY   "test<@TYPE@>();\n"
#          HEADER "extern template void test<@TYPE@>();\n"
#          FILE   test.cc.in)
#
#    The ``NAME`` ist optional and defaults to ``DEFAULT``.  ``BODY`` and
#    ``HEADER`` are string templates that will be sent through
#    ``string(CONFIGURE ...)`` to generate content for the body of ``main()``
#    and for the header, respectively.  ``FILE`` is the name of a template
#    file that gets sent through ``configure_file()`` and might look something
#    like this::
#
#      // @GENERATED_SOURCE@
#
#      #include <config.h>
#
#      #include "mytest.hh" // extern declarations
#
#      template void test<@TYPE@>();
#
#    The template strings and files contain ``@VAR@`` or ``${VAR}``-style
#    variable substitutions that will be replace upon instanciation.
#
#    These templates can then be used to generate instances::
#
#      begin_instances(HEADER mytest.hh.in BODY mytest.cc.in OUTPUTS SOURCES_LIST)
#
#      foreach(TYPE IN ITEMS int unsigned std::complex<float>)
#        add_instance(TEMPLATE mytemplate NAME "${TYPE}")
#      endforeach(TYPE IN ITEMS int unsigned std::complex<float>)
#
#      end_instances()
#
#    ``begin_instance()`` sets up some variables and remembers the templates
#    for header and the main ``.cc`` file, as well as where to save the list
#    of generated instance ``.cc`` files (``SOURCES_LIST`` in this case).
#
#    Then the variable ``TYPE`` is set up as part of the ``foreach()``, and
#    ``add_instance()`` is called to generate one instance with the variables
#    that are currently in scope.  ``NAME`` is used as a part of the output
#    filename after mangling, i.e. for ``std::complex<float>`` the output
#    filename would be ``test_std_complex_float.cc``.  The result of
#    instantiating the ``HEADER`` and ``BODY`` template is collected.
#
#    Finally, ``end_instances()`` is called.  This puts the collected
#    ``HEADER`` and ``BODY`` content into the variables ``HEADER_CONTENT`` and
#    ``BODY_CONTENT`` so it is available for substitution.  It then generates
#    ``mytest.hh`` from ``mytest.hh.in`` and ``mytest.cc` from
#    ``mytest.cc.in``.  Lastly, it sets ``SOURCES_LIST`` to the list of
#    generated instance ``.cc`` files.
#
#    The definition of the actual test in ``CMakeLists.txt`` might then look
#    something like::
#
#      dune_add_test(NAME mytest
#        SOURCES mytest.cc ${SOURCES_LIST}
#        LINK_LIBRARIES dunecommon
#      )
#
#    Here is what ``mytest.hh.in`` might look like::
#
#      // @GENERATED_SOURCE@
#      #ifndef MYTEST_HH
#      #define MYTEST_HH
#
#      #include <test.hh>
#
#      @HEADER_CONTENT@
#
#      #endif // MYTEST_HH
#
#    And here is ``mytest.cc.in``::
#
#      // @GENERATED_SOURCE@
#      #include <config.h>
#
#      #include "mytest.hh"
#
#      int main() {
#        @BODY_CONTENT@
#      }
#
#    .. rubric:: How instantiation happens
#
#    Whenever a template file or string template is instanciated (using
#    ``configure_file(...)`` or ``string(CONFIGURE ...)`` the currently
#    visible variables are substituted.  This means you can call
#    :ref:`add_instance` in a ``foreach()`` loop and substitute the template
#    can use the loop variable.  You can also do ``set(VAR ...)``, and ``VAR``
#    will be visible to the template.
#
#    The cmake functions that do template instanciation start the names of
#    their internal variables with an underscore ``_``, so it is best not to
#    start the variables that you want to substitute with underscores.  Also,
#    variables beggining with ``GENERATED_INSTANCES_`` are used to store
#    templates and communicate between functions, so stay away from them as
#    well.
#
#    For convenience, whenever any of the functions generate a file, it will
#    first set the variable ``GENERATED_SOURCE`` using
#    :ref:`set_generated_source` so the generated file can be marked as such.
#    :ref:`end_instances` will also set ``HEADER_CONTENT`` and
#    ``BODY_CONTENT`` to the accumulated content of header and body.  This is
#    done before either the header or the body is generated, so you can use
#    both variables in both instantiations.
#
# .. cmake_function:: begin_instances
#
#    .. cmake_brief::
#
#       Prepare for a list of instances.
#
#    .. cmake_param:: HEADER
#       :single:
#
#       Header file template to instantiate, e.g. ``mytest.hh.in``.  If not
#       given, no header is instantiated.
#
#    .. cmake_param:: BODY
#       :single:
#
#       File to instantiate with body content, e.g. ``mytest.cc.in``.  If not
#       given, no file is instantiated.
#
#    .. cmake_param:: OUTPUTS
#       :single:
#       :argname: outputs_var
#
#       Name of a cmake variable in the scope of the caller to store the list
#       of generated instance files in.
#
#    Clear the internal variables that hold the accumulated header and body
#    content, as well as the list of generated files.  Remember the parameters
#    given for :ref:`end_instances`.
#
#    .. note::
#
#       There should be a matching :ref:`end_instances`.  Since information is
#       communicated through variables in the callers scope,
#       ``begin_instances()``/``end_instances()`` blocks may not be nested
#       inside the same scope.  Since a function is a new scope, it may safely
#       contain a ``begin_instances()``/``end_instances()`` block, even if it
#       is itself called from one.
#
#
# .. cmake_function:: end_instances
#
#    .. cmake_brief::
#
#       Finalize a list of instances.
#
#    Set ``HEADER_CONTENT`` and ``BODY_CONTENT`` to the accumulated header and
#    body content, respectively.  Then generate the header and the body files
#    given to the matching :ref:`begin_instances`.  Set ``outputs_var`` (given
#    to :ref:`begin_instances`) to the list of generated instance files (not
#    including the header and the body file).
#
#    Before generating any file, set up the variable ``GENERATED_SOURCE``
#    using :ref:`set_generated_source` so it is available during generation.
#
#
# .. cmake_function:: define_template
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
#    .. cmake_param:: BODY
#       :single:
#
#       Template string for the body content.  Defaults empty.
#
#    .. cmake_param:: HEADER
#       :single:
#
#       Template string for the header content.  Defaults empty.
#
#    .. cmake_param:: FILE
#       :single:
#
#       File template name (such as ``test.cc.in``) to instantiate for each
#       instance.  Defaults empty; nothing is generated when empty.
#
#    Define a template for use in :ref:`add_instance`.  The template is stored
#    in cmake variables in the scope of the caller.  Calls to
#    ``define_template()`` may appear anywhere; i.e. inside or outside an
#    ``begin_instances()``/``end_instances()`` block, in a parent scope,
#    between calls to :ref:`add_instance`, etc.  The only requirement is that
#    a template has been defined before it is used in :ref:`add_instance`, and
#    that the variables holding the template definition are in scope when it
#    is used in :ref:`add_instance`.
#
#    It is permissible to redefine an existing template, this overwrites all
#    sub-templates, not just the ones provided.
#
#
# .. cmake_function:: add_instance
#
#    .. cmake_brief::
#
#       Add an instance to the current block.
#
#    .. cmake_param:: NAME
#       :single:
#
#       Used to build the name of the generated file when ``OUTPUT`` is not
#       given.  The output file is built from this parameter and the name of
#       the template file by first removing any ``.in`` suffix from the
#       filename and then insterspersing a mangled version of ``name`` between
#       base name and suffix of what remains.  I.e. if the template file name
#       is ``test.cc.in`` and ``name`` is given as ``std::complex<float>``,
#       then the output file name will be generated as
#       ``test_std_complex_float.cc``.
#
#    .. cmake_param:: OUTPUT
#       :single:
#
#       Explicitly set the output filename.  This filename is not mangled.
#
#    .. cmake_param:: TEMPLATE
#       :single:
#
#       Set the name of the to use.  Defaults to ``DEFAULT``.
#
#    If the template has a file name set, use it to generate an instance file
#    with the currently visible variables.  The name of the generated file can
#    be explicitly specified using ``OUTPUT`` or it can be hauristically
#    generated from the template file name and whatever was provided in
#    ``NAME``.  Append the name of the generated file to the internal list of
#    generated files.
#
#    Before generating the file, call :ref:`set_generated_source` to set up
#    the variable ``GENERATED_SOURCE``.
#
#    Then, generate contents for the header and the body using the template
#    and accumulate it in internal storage for later use in
#    :ref:`end_instances`.
#
#
# .. cmake_function:: set_generated_source
#
#    .. cmake_brief::
#
#       Generate boilerplate to mark a file as generated.
#
#    .. cmake_param:: from
#       :positional:
#       :single:
#       :required:
#
#       The name of the template file that this file was generated from.
#
#    Set the variable ``GENERATED_SOURCE`` in the callers scope to a message
#    that
#
#    * tellsa human reader that this file was generated, and what it was
#      generated from,
#
#    * tells editors to mark the file as read-only, so that noone
#      accidentially edits and loses those edits the next time the file is
#      regenerated.
#
#    The intended use is to put something like this in the first line of
#    template files::
#
#      // @GENERATED_SOURCE@
#
#    This will expand to something like this::
#
#      // generated from template.cc.in by cmake   -*- buffer-read-only:t -*- vim: set readonly:
#
#    .. note::
#
#       This really should be the first line, or for scripts that start with a
#       ``#!`` it should be the second line.  Otherwise the editor hints
#       aren't as likely to work.


function(begin_instances)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "" # options
    "HEADER;BODY;OUTPUTS" # one_value_keywords
    "" # multi_value_keywords
    )
  if(DEFINED arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "begin_instances() called with unrecognized arguments: "
      "${arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED arg_UNPARSED_ARGUMENTS)

  set(GENERATED_INSTANCES_FILES          "" PARENT_SCOPE)
  set(GENERATED_INSTANCES_HEADER_CONTENT "" PARENT_SCOPE)
  set(GENERATED_INSTANCES_BODY_CONTENT   "" PARENT_SCOPE)

  set(GENERATED_INSTANCES_HEADER  "${arg_HEADER}"  PARENT_SCOPE)
  set(GENERATED_INSTANCES_BODY    "${arg_BODY}"    PARENT_SCOPE)
  set(GENERATED_INSTANCES_OUTPUTS "${arg_OUTPUTS}" PARENT_SCOPE)
endfunction(begin_instances)


function(end_instances)
  if(ARGC GREATER 0)
    message(FATAL_ERROR "end_instances() called with unrecognized arguments: "
      "${ARGV}")
  endif(ARGC GREATER 0)

  # set variables
  set(HEADER_CONTENT "${GENERATED_INSTANCES_HEADER_CONTENT}")
  set(BODY_CONTENT "${GENERATED_INSTANCES_BODY_CONTENT}")
  if(NOT (GENERATED_INSTANCES_OUTPUTS STREQUAL ""))
    set("${GENERATED_INSTANCES_OUTPUTS}" "${GENERATED_INSTANCES_FILES}" PARENT_SCOPE)
    set("${GENERATED_INSTANCES_OUTPUTS}" "${GENERATED_INSTANCES_FILES}")
  endif(NOT (GENERATED_INSTANCES_OUTPUTS STREQUAL ""))

  # generate header
  if(NOT (GENERATED_INSTANCES_HEADER STREQUAL "" ))
    set_generated_source("${GENERATED_INSTANCES_HEADER}.in")
    message(STATUS "Generating ${GENERATED_INSTANCES_HEADER}")
    configure_file("${GENERATED_INSTANCES_HEADER}.in" "${GENERATED_INSTANCES_HEADER}")
    set_property(SOURCE "${GENERATED_INSTANCES_HEADER}" PROPERTY GENERATED TRUE)
  endif(NOT (GENERATED_INSTANCES_HEADER STREQUAL "" ))

  # generate body
  if(NOT (GENERATED_INSTANCES_BODY STREQUAL ""))
    set_generated_source("${GENERATED_INSTANCES_BODY}.in")
    message(STATUS "Generating ${GENERATED_INSTANCES_BODY}")
    configure_file("${GENERATED_INSTANCES_BODY}.in" "${GENERATED_INSTANCES_BODY}")
    set_property(SOURCE "${GENERATED_INSTANCES_BODY}" PROPERTY GENERATED TRUE)
  endif(NOT (GENERATED_INSTANCES_BODY STREQUAL ""))
endfunction(end_instances)


function(define_template)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "" # options
    "NAME;BODY;HEADER;FILE" # one_value_keywords
    "" # multi_value_keywords
    )

  if(DEFINED arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "define_template() called with unrecognized arguments: "
      "${arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED arg_UNPARSED_ARGUMENTS)

  if(NOT DEFINED arg_NAME)
    set(arg_NAME DEFAULT)
  endif(NOT DEFINED arg_NAME)

  set("GENERATED_INSTANCES_TEMPLATE_${arg_NAME}_BODY"   "${arg_BODY}"   PARENT_SCOPE)
  set("GENERATED_INSTANCES_TEMPLATE_${arg_NAME}_HEADER" "${arg_HEADER}" PARENT_SCOPE)
  set("GENERATED_INSTANCES_TEMPLATE_${arg_NAME}_FILE"   "${arg_FILE}"   PARENT_SCOPE)
endfunction(define_template)


# build output file name: mangle the NAME by replacing anything special with
# "_" and intersperse the result between basename and extension of the
# TEMPLATE, after stripping any ".in" from it's end
function(_make_instance_file_name template name result)
  # strip ".in"
  string(REGEX REPLACE "\\.in\$" "" base "${template}")

  # split into prefix and suffix
  if(base MATCHES "\\.")
    string(REGEX REPLACE "\\.[^.]*\$"      ""    prefix "${base}")
    string(REGEX REPLACE "^.*(\\.[^.]*)\$" "\\1" suffix "${base}")
  else(base MATCHES "\\.")
    set(prefix "${base}")
    set(suffix)
  endif(base MATCHES "\\.")

  # mangle the name
  string(REGEX REPLACE "[^a-zA-Z0-9]+" "_" mangled_name "${name}")
  string(REGEX REPLACE "^_+"           ""  mangled_name "${mangled_name}")
  string(REGEX REPLACE "_+\$"          ""  mangled_name "${mangled_name}")
  if(mangled_name STREQUAL "")
    message(FATAL_ERROR "\"${name}\" is empty after mangling")
  endif(mangled_name STREQUAL "")

  set("${result}" "${prefix}_${mangled_name}${suffix}" PARENT_SCOPE)
endfunction(_make_instance_file_name)


function(set_generated_source from)
  set(GENERATED_SOURCE
    "generated from ${from} by cmake   -*- buffer-read-only:t -*- vim: set readonly:"
    PARENT_SCOPE)
endfunction(set_generated_source)


function(add_instance)
  cmake_parse_arguments(PARSE_ARGV 0 _arg
    "" # options
    "NAME;OUTPUT;TEMPLATE" # one_value_keywords
    "" # multi_value_keywords
    )

  if(DEFINED _arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "add_instance() called with unrecognized arguments: "
      "${_arg_UNPARSED_ARGUMENTS}")
  endif(DEFINED _arg_UNPARSED_ARGUMENTS)

  #determine template to use
  if(_arg_TEMPLATE STREQUAL "")
    set(_arg_TEMPLATE DEFAULT)
  endif(_arg_TEMPLATE STREQUAL "")
  set(_body_template   "${GENERATED_INSTANCES_TEMPLATE_${_arg_TEMPLATE}_BODY}")
  set(_header_template "${GENERATED_INSTANCES_TEMPLATE_${_arg_TEMPLATE}_HEADER}")
  set(_template_file   "${GENERATED_INSTANCES_TEMPLATE_${_arg_TEMPLATE}_FILE}")

  if(NOT DEFINED _arg_OUTPUT)
    _make_instance_file_name("${_template_file}" "${_arg_NAME}" _arg_OUTPUT)
  endif(NOT DEFINED _arg_OUTPUT)

  set_generated_source("${_template_file}")

  # generate instanciation
  message(STATUS "Generating ${_arg_OUTPUT}")
  configure_file("${_template_file}" "${_arg_OUTPUT}")
  set_property(SOURCE "${_arg_OUTPUT}" PROPERTY GENERATED TRUE)
  set(GENERATED_INSTANCES_FILES "${GENERATED_INSTANCES_FILES};${_arg_OUTPUT}"
    PARENT_SCOPE)

  # append header content
  string(CONFIGURE "${_header_template}" _header_content)
  set(GENERATED_INSTANCES_HEADER_CONTENT
    "${GENERATED_INSTANCES_HEADER_CONTENT}${_header_content}"
    PARENT_SCOPE)

  # append body content
  string(CONFIGURE "${_body_template}" _body_content)
  set(GENERATED_INSTANCES_BODY_CONTENT
    "${GENERATED_INSTANCES_BODY_CONTENT}${_body_content}"
    PARENT_SCOPE)
endfunction(add_instance)
