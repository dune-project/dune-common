#[=======================================================================[.rst:
DuneDeclareTestLabel
--------------------

.. cmake_function:: dune_declare_test_label

  .. cmake_brief::

      Declare labels for :ref:`dune_add_test`.

  .. cmake_param:: LABELS
      :multi:

      The names of labels to declare.  Label names must be nonempty and
      consist only of alphanumeric characters plus :code:`-` and :code:`_`
      to make sure it is easy to construct regular expressions from them for
      :code:`ctest -L ${label_regex}`.

  Labels need to be declared to ensure that the target
  :code:`build_${label}_tests` exists.  They will normally be declared
  on-demand by :ref:`dune_add_test`.  But sometimes it is useful to be able to
  run :code:`make build_${label}_tests` whether or not any tests with that
  label exists in a module.  For these cases :ref:`dune_declare_test_label` can
  be called explicitly.

#]=======================================================================]

include_guard(GLOBAL)

function (dune_declare_test_label)
  include(CMakeParseArguments)
  set(OPTIONS)
  set(SINGLEARGS)
  set(MULTIARGS LABELS)
  cmake_parse_arguments(arg "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  if ( (DEFINED arg_UNPARSED_ARGUMENTS) AND NOT ( arg_UNPARSED_ARGUMENTS STREQUAL "" ) )
    message(FATAL_ERROR "Unhandled extra arguments given to dune_declare_test_label(): "
      "<${arg_UNPARSED_ARGUMENTS}>")
  endif ()

  foreach (label IN LISTS arg_LABELS)
    # Make sure the label is not empty, and does not contain any funny
    # characters, in particular regex characters
    if (NOT (label MATCHES "[-_0-9a-zA-Z]+"))
      message(FATAL_ERROR "Refusing to add label \"${label}\" since it is "
        "empty or contains funny characters (characters other than "
        "alphanumeric ones and \"-\" or \"_\"; the intent of this restriction "
        "is to make construction of the argument to \"ctest -L\" easier")
    endif ()
    set (target "build_${label}_tests")
    if (NOT TARGET "${target}")
      add_custom_target("${target}")
    endif ()
  endforeach ()
endfunction (dune_declare_test_label)
