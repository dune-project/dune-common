# Module with backward compatibility implementation of newer cmake functionality
#
# .. cmake_module::
#
#    This module contains backward compatibility implementations of cmake
#    functionality that is not available in all cmake versions we support.
#
#    * :ref:`dune_list_filter(...) <dune_list_filter>` for ``list(FILTER
#      ...)`` from cmake 3.7
#
#
# .. cmake_function:: dune_list_filter
#
#    .. cmake_brief::
#
#       Compatibility implementation of ``list(FILTER)``
#
#    .. cmake_param:: list
#       :positional:
#       :single:
#       :required:
#
#       Name of list variable used as both input and output.
#
#    .. cmake_param:: <INCLUDE|EXCLUDE>
#       :positional:
#       :option:
#       :required:
#
#       Whether to include or to exclude the items matching the regular
#       expression.
#
#    .. cmake_param:: REGEX
#       :single:
#       :required:
#       :argname: regular_expression
#
#       The regular expression to match the items against.
#
#    Match each item in the list against the regular expression.  In
#    ``INCLUDE`` mode the result contains all items that matched, in
#    ``EXCLUDE`` mode it contains all items that did not match.  Store the
#    result back in the variable ``list`` in the scope of the caller.
#
#    This is exactly the same as the ``list(FILTER ...)`` command available in
#    cmake 3.7 and onward.
include_guard(GLOBAL)

# list(FILTER...) was introduced in cmake 3.6, this is a compatibility
# implementation for earlier cmakes
function(dune_list_filter list mode REGEX regular_expression)
  message(DEPRECATION "dune_list_filter is deprecated and will be removed after Dune 2.8. Use list(FILTER ...) from CMake 3.6")

  # validate arguments
  if(NOT (("${mode}" STREQUAL "INCLUDE") OR ("${mode}" STREQUAL "EXCLUDE")))
    message(FATAL_ERROR "unsupported mode '${mode}', must be either INCLUDE or EXCLUDE")
  endif()
  if(NOT ("${REGEX}" STREQUAL "REGEX"))
    message(FATAL_ERROR "dune_list_filter can only filter by regular expression")
  endif()
  if("${ARGC}" GREATER 4)
    message(FATAL_ERROR "extra arguments given: <${ARGN}>")
  endif()

  # cmake can't destinguish between empty lists and lists with one empty
  # element.  This is a problem when consecutively appending elements to a
  # list: if the first elements we append are empty, we loose them.  The
  # "non-empty" token makes sure we start with a non-empty list and avoid this
  # problem.
  set(matched "non-empty")
  set(unmatched "non-empty")
  foreach(item IN LISTS "${list}")
    # list(APPEND) does not quote the appended item (as of cmake 3.7.2), so do
    # it manually
    string(REPLACE [[;]] [[\;]] quoted_item "${item}")
    if("${item}" MATCHES "${regular_expression}")
      list(APPEND matched "${quoted_item}")
    else()
      list(APPEND unmatched "${quoted_item}")
    endif()
  endforeach(item)

  if("${mode}" STREQUAL "INCLUDE")
    set(result "${matched}")
  else()
    set(result "${unmatched}")
  endif()

  # remove the non-empty token from above.  If the proper result would be a
  # list of one empty element, we have no way of preserving that, it will turn
  # into an empty list.
  string(REGEX REPLACE "^non-empty;?" "" result "${result}")

  # export
  set("${list}" "${result}" PARENT_SCOPE)
endfunction(dune_list_filter)
