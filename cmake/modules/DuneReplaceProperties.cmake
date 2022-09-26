# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneReplaceProperties
---------------------

Utility functions to modify target properties

.. cmake:command:: replace_properties

.. code-block:: cmake

  replace_properties(<replacements>...
    [GLOBAL]
    [DIRECTORY <dir>]
    [PROPERTY <prop>]
    [TARGET <targets>...]
    [SOURCE <sources>...]
    [TEST <tests>...]
    [CACHE <cache>...]
  )

#]=======================================================================]
include_guard(GLOBAL)


function(replace_properties)
  set(_first_opts "GLOBAL;DIRECTORY;TARGET;SOURCE;CACHE")
  cmake_parse_arguments(REPLACE "GLOBAL"
    "DIRECTORY;PROPERTY" "TARGET;SOURCE;TEST;CACHE" ${ARGN})

  set(MY_DIRECTORY TRUE)
  foreach(i ${_first_opts})
    if(REPLACE_${i})
      set(MY_DIRECTORY FALSE)
    endif()
  endforeach()
  if(NOT MY_DIRECTORY)
    list(FIND REPLACE_UNPARSED_ARGUMENTS DIRECTORY _found)
    if(_found GREATER -1)
      list(REMOVE_AT REPLACE_UNPARSED_ARGUMENTS ${_found})
      set(MY_DIRECTORY TRUE)
      set(REPLACE_DIRECTORY "")
    endif()
  endif()

  # setup options
  if(REPLACE_GLOBAL)
    set(option_command GLOBAL)
  elseif(MY_DIRECTORY)
    set(option_command DIRECTORY)
  elseif(REPLACE_DIRECTORY)
    set(option_command DIRECTORY)
    set(option_arg ${REPLACE_DIRECTORY})
  elseif(REPLACE_TARGET)
    set(option_command TARGET)
    set(option_arg ${REPLACE_TARGET})
  elseif(REPLACE_SOURCE)
    set(option_command SOURCE)
    set(option_arg ${REPLACE_SOURCE})
  elseif(REPLACE_TEST)
    set(option_command TEST)
    set(option_arg${REPLACE_TEST})
  elseif(REPLACE_CACHE)
    set(option_command CACHE)
    set(option_arg ${REPLACE_CACHE})
  endif()

  if(NOT (REPLACE_CACHE OR REPLACE_TEST OR REPLACE_SOURCE
      OR REPLACE_TARGET OR REPLACE_DIRECTORY OR REPLACE_GLOBAL
      OR MY_DIRECTORY))
    message(ERROR "One of GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, or CACHE"
      " has to be present")
  endif()

  list(LENGTH REPLACE_UNPARSED_ARGUMENTS length)
#  if(NOT (REPLACE_GLOBAL AND REPLACE_TARGET AND
#        REPLACE_SOURCE AND REPLACE
  math(EXPR mlength "${length} % 2 ")
  math(EXPR hlength "${length} / 2 - 1")

  if(NOT ${mlength} EQUAL 0)
    message(ERROR "You need to specify pairs consisting of a regular expression and a replacement string.")
  endif()

  if(NOT length GREATER 0)
    message(ERROR "You need to specify at least on pair consisting of a regular expression
and a replacement string. ${REPLACE_UNPARSED_ARGUMENTS}")
  endif()

  foreach(_target ${option_arg})
    replace_properties_for_one()
  endforeach()

  list(LENGTH option_arg _length)
  if(_length EQUAL 0)
    replace_properties_for_one()
  endif()
endfunction(replace_properties)


# ------------------------------------------------------------------------
# Internal macros
# ------------------------------------------------------------------------


macro(replace_properties_for_one)
  get_property(properties ${option_command} ${_target}
    PROPERTY ${REPLACE_PROPERTY})
  if(NOT properties)
    # property not set. set it directly
    foreach(i RANGE 0 ${hlength})
      math(EXPR idx "(2 * ${i}) + 1")
      list(GET REPLACE_UNPARSED_ARGUMENTS ${idx} repl)
      list(APPEND replacement ${repl})
    endforeach()
    list(REMOVE_DUPLICATES replacement)
    set_property(${option_command} ${_target} ${REPLACE_APPEND}
      ${REPLACE_APPEND_STRING} PROPERTY ${REPLACE_PROPERTY} ${replacement})
  else()
    foreach(prop ${properties})
      set(matched FALSE)
      foreach(i RANGE 0 ${hlength})
        math(EXPR regexi "2 * ${i}")
        math(EXPR repli  "${regexi} +1")
        list(GET REPLACE_UNPARSED_ARGUMENTS ${regexi} regex)
        list(GET REPLACE_UNPARSED_ARGUMENTS ${repli} replacement)
        string(REGEX MATCH ${regex} match ${prop})

        if(match)
          list(APPEND new_props ${replacement})
          set(matched TRUE)
        endif()
      endforeach()

      if(NOT matched)
        list(APPEND new_props ${prop})
      endif()
    endforeach()
    list(REMOVE_DUPLICATES new_props)
    set_property(${option_command} ${_target}
      PROPERTY ${REPLACE_PROPERTY} ${new_props})
  endif()
  get_property(properties ${option_command} ${_target} PROPERTY ${REPLACE_PROPERTY})
endmacro(replace_properties_for_one)
