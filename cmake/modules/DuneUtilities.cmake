# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneUtilities
-------------

Utility functions used by multiple files.

.. cmake:command:: dune_module_to_uppercase

  Convert a module name into an uppercase string.

  .. code-block:: cmake

    dune_module_to_uppercase(<upper> <module>)

  Convert a module name given by ``<module>`` parameter into an uppercase
  string stored in the output variable ``<upper>`` where all dashes ``-``
  are replaced by underscores ``_``.

  Example: ``dune-common -> DUNE_COMMON``


.. cmake:command:: dune_module_to_macro

  Convert a module name into a capitalized string.

  .. code-block:: cmake

    dune_module_to_macro(<macroname> <module>)

  Convert a module name given by ``<module>`` into a string stored in the
  output variable ``<macroname>`` where all dashes ``-`` are removed and the
  first letter and all letters following a dash are capitalized.

  Example: ``dune-grid-howto -> DuneGridHowto``

.. cmake:command:: dune_deprecate_variable

  Writes a deprecation message on a variable read access. This is intended to be
  called by ``variable_watch(....)``.

  .. code-block:: cmake

    variable_watch(<deprecared-variable> dune_deprecate_variable)


#]=======================================================================]
include_guard(GLOBAL)


# Converts a module name given by _module into an uppercase string
# _upper where all dashes (-) are replaced by underscores (_)
# Example: dune-common -> DUNE_COMMON
macro(dune_module_to_uppercase _upper _module)
  string(TOUPPER "${_module}" ${_upper})
  string(REPLACE "-" "_" ${_upper} "${${_upper}}")
endmacro(dune_module_to_uppercase _upper _module)


# Converts a module name given by _dune_module into a string _macro_name
# where all dashes (-) are removed and letters after a dash are capitalized
# Example: dune-grid-howto -> DuneGridHowto
macro(dune_module_to_macro _macro_name _dune_module)
  set(${_macro_name} "")
  set(_rest "${_dune_module}")
  string(FIND "${_rest}" "-" _found)
  while(_found GREATER -1)
    string(REGEX REPLACE "([^-]*)-.*" "\\1" _first_part
      "${_rest}")
    string(REGEX REPLACE "[^-]*-(.*)" "\\1" _rest
      "${_rest}")
    string(SUBSTRING "${_first_part}" 0 1 _first_letter)
    string(SUBSTRING "${_first_part}" 1 -1 _rest_first_part)
    string(TOUPPER "${_first_letter}" _first_letter)
    set(${_macro_name} "${${_macro_name}}${_first_letter}${_rest_first_part}")
    string(FIND "${_rest}" "-" _found)
  endwhile()
  string(LENGTH "${_rest}" _length)
  string(SUBSTRING "${_rest}" 0 1 _first_letter)
  string(SUBSTRING "${_rest}" 1 -1 _rest)
  string(TOUPPER "${_first_letter}" _first_letter)
  set(${_macro_name} "${${_macro_name}}${_first_letter}${_rest}")
endmacro(dune_module_to_macro _macro_name _dune_module)

function(dune_deprecate_variable _variable _access)
  if(_access STREQUAL "READ_ACCESS")
    message(DEPRECATION "Variable ${_variable} is deprecated")
  endif()
endfunction()


### Internal handling of ALL_DEPENDENCIES deprecation. Remove after Dune 2.12

define_property(GLOBAL PROPERTY DUNE_DISABLE_ALL_DEPENDENCIES_DEPRECATION_WARNING
  BRIEF_DOCS "Internal DUNE warning handler of ALL_DEPENDENCIES. DO NOT EDIT!"
  FULL_DOCS "Internal DUNE warning handler of ALL_DEPENDENCIES. DO NOT EDIT!")
set_property(GLOBAL PROPERTY DUNE_DISABLE_ALL_DEPENDENCIES_DEPRECATION_WARNING OFF)

function(dune_deprecate_ALL_DEPENDENCIES _variable _access)
  get_property(_prop GLOBAL PROPERTY DUNE_DISABLE_ALL_DEPENDENCIES_DEPRECATION_WARNING)
  if(NOT _prop)
    dune_deprecate_variable(${_variable} ${_access})
  endif()
endfunction()

variable_watch(ALL_DEPENDENCIES dune_deprecate_ALL_DEPENDENCIES)
