# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePolicy
----------

Get and set dune policies to control changes in behavior of the dune build
system. A policy is by default set to `OLD` if not set otherwise, except
if the policy's activation version is reached. Each policy can specify
when it is set automatically to `NEW`. The default behavior can be
influenced by the global cmake variable ``DUNE_POLICY_DEFAULT``.

.. cmake:command:: dune_policy

  .. code-block:: cmake

    dune_policy(GET <policy> <var>)
    dune_policy(SET <policy> <value>)
    dune_policy(LIST)

.. cmake:command:: dune_define_policy

  .. code-block:: cmake

    dune_define_policy(<policy> <module> <version> "<doc>")

  Introduces a `<policy>`, a name that identifies a change in behavior
  of the dune build-system. It is associated to a dune `<module>` and
  is set to "NEW" automatically, if that module reaches a given
  `<version>`. The documentation `<doc>` is shown if the policy is not
  set by the module author and the automatic activation version is not
  reached.

Global options
^^^^^^^^^^^^^^

``DUNE_POLICY_DEFAULT`` (default="OLD")
  The default value of an unset policy. Either `OLD` (default) or `NEW`.

``DUNE_POLICY_DISABLE_WARNING`` (default=FALSE)
  If set to `TRUE`, warnings about unset dune policies are deactivated.

``DUNE_POLICY_IGNORE_UNDEFINED`` (default=FALSE)
  Ignore undefined dune policies in GET and SET operations.
  This option is useful if you want to set a policy in a module that
  should be compatible with a range of upstream dune modules where
  possibly not all policies are already defined. If this option is set to
  `FALSE`, an error message is shown if you try to set or get an undefined
  policy.

#]=======================================================================]
include_guard(GLOBAL)

set(DUNE_POLICY_DEFAULT "OLD" CACHE STRING "Default value for an unset dune policy.")
set_property(CACHE DUNE_POLICY_DEFAULT PROPERTY STRINGS "OLD" "NEW")
option(DUNE_POLICY_DISABLE_WARNING "Do not show warnings about unset dune policies." FALSE)
option(DUNE_POLICY_IGNORE_UNDEFINED "Ignore undefined dune policies in GET and SET operations." FALSE)

# print a help message with the signature of the dune_policy function
function(dune_policy_help _errorlevel _msg)
  message(${_errorlevel} "${_msg}"
    "The function `dune_policy` has the following signatures:"
    "  dune_policy(GET <policy> <var>): extract the value of the given <policy> in the variable <var>."
    "  dune_policy(SET <policy> <value>): change the given <policy> to <value>."
    "  dune_policy(LIST): list all registered policies with their values."
    "  dune_policy(HELP): show this help message.")
endfunction(dune_policy_help)

# get the value of a _policy, or the DUNE_POLICY_DEFAULT, or NEW if the policy's dune version is reached
function(dune_get_policy _policy _var)
  get_property(_policy_defined GLOBAL PROPERTY DUNE_POLICY_${_policy} DEFINED)
  if(NOT _policy_defined)
    if(DUNE_POLICY_IGNORE_UNDEFINED)
      return()
    else()
      dune_policy_help(FATAL_ERROR "Undefined policy ${_policy}.")
    endif()
  endif()
  get_property(_policy_set GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy} SET)
  if(NOT _policy_set)
    get_property(_policy_version GLOBAL PROPERTY DUNE_POLICY_${_policy}_VERSION)
    get_property(_policy_module GLOBAL PROPERTY DUNE_POLICY_${_policy}_MODULE)
    if(${_policy_module}_VERSION VERSION_GREATER_EQUAL _policy_version)
      set(_policy_value "NEW")
    else()
      get_property(_policy_doc GLOBAL PROPERTY DUNE_POLICY_${_policy} BRIEF_DOCS)
      get_property(_policy_warning GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy}_WARNING)
      if(NOT _policy_warning AND NOT DUNE_POLICY_DISABLE_WARNING)
        message(AUTHOR_WARNING "Policy ${_policy} is not set: ${_policy_doc} "
          "Use the dune_policy(SET) command to set the policy and suppress this warning. "
          "The default value ${DUNE_POLICY_DEFAULT} will be used in the meantime.")
        set_property(GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy}_WARNING TRUE)
      endif()
      set(_policy_value ${DUNE_POLICY_DEFAULT})
    endif()
  else()
    get_property(_policy_value GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy})
  endif()
  set(${_var} ${_policy_value} PARENT_SCOPE)
endfunction(dune_get_policy)

# set a given _policy to the _value
function(dune_set_policy _policy _value)
  get_property(_policy_defined GLOBAL PROPERTY DUNE_POLICY_${_policy} DEFINED)
  if(NOT _policy_defined)
    if(DUNE_POLICY_IGNORE_UNDEFINED)
      return()
    else()
      dune_policy_help(FATAL_ERROR "Undefined policy ${_policy}.")
    endif()
  endif()
  set_property(GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy} ${_value})
endfunction(dune_set_policy)

# generic policy method to get, set, or list policies
function(dune_policy _method)
  if(_method STREQUAL "GET")
    if(ARGV2 LESS 3)
      dune_policy_help(FATAL_ERROR "Not enough arguments passed to dune_policy(GET...)")
    endif()
    dune_get_policy(${ARGV1} _var)
    set(${ARGV2} ${_var} PARENT_SCOPE)
  elseif(_method STREQUAL "SET")
    if(ARGV2 LESS 3)
      dune_policy_help(FATAL_ERROR "Not enough arguments passed to dune_policy(SET...)")
    endif()
    dune_set_policy(${ARGV1} ${ARGV2})
  elseif(_method STREQUAL "LIST")
    get_property(_policies GLOBAL PROPERTY DUNE_POLICIES)
    foreach(_policy ${policies})
      get_property(_policy_value GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy})
      if(NOT _policy_value)
        set(_policy_value ${DUNE_POLICY_DEFAULT})
      endif()
      message(STATUS "dune_policy(${_policy}) = ${_policy_value}")
    endforeach(_policy)
  elseif(_method STREQUAL "HELP")
    dune_policy_help(STATUS "How to use dune_policy?")
  else()
    dune_policy_help(FATAL_ERROR "Unknown first argument to dune_policy().")
  endif()
endfunction(dune_policy)

# introduce a new policy into the system
function(dune_define_policy _policy _module _version _doc)
  define_property(GLOBAL PROPERTY DUNE_POLICY_${_policy}
    BRIEF_DOCS "${_doc}" FULL_DOCS "${_doc}")
  set_property(GLOBAL PROPERTY DUNE_POLICY_${_policy}_MODULE ${_module})
  set_property(GLOBAL PROPERTY DUNE_POLICY_${_policy}_VERSION ${_version})
  set_property(GLOBAL APPEND PROPERTY DUNE_POLICIES ${_policy})
endfunction(dune_define_policy)
