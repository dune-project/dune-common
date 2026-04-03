# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePolicy
----------

Get and set dune policies to control changes in behavior of the dune build
system. A policy is by default set to ``OLD`` if not set otherwise. The
default behavior can be influenced by the global CMake variable
``DUNE_POLICY_DEFAULT``.

.. cmake:command:: dune_policy

  Get, set, list, or explain DUNE buildsystem policies.

  .. code-block:: cmake

    dune_policy(GET <policy> <var>)
    dune_policy(SET <policy> <value> [QUIET])
    dune_policy(LIST)
    dune_policy(HELP)

  ``GET <policy> <var>``
    Retrieve the effective value of ``<policy>`` into ``<var>``.

  ``SET <policy> <value> [QUIET]``
    Set ``<policy>`` to ``OLD`` or ``NEW`` for the current project. If
    ``QUIET`` is given, undefined policies are ignored without a warning.

  ``LIST``
    Print the registered policies and their current values.

  ``HELP``
    Print a short usage summary for :cmake:command:`dune_policy()`.

.. cmake:command:: dune_define_policy

  Register a new DUNE buildsystem policy.

  .. code-block:: cmake

    dune_define_policy(<policy> "<doc>")
    dune_define_policy(<policy> <module> <version> "<doc>")

  ``<policy>``
    Policy identifier naming a specific behavior change.

  ``<doc>``
    Documentation shown when the policy is unset and
    ``DUNE_POLICY_DISABLE_WARNING`` is not set to ``TRUE``.

  ``<module>``, ``<version>``
    Deprecated compatibility arguments. They are ignored.

  .. deprecated:: 2.12
     The signature ``dune_define_policy(<policy> <module> <version> <doc>)``
     is deprecated. Use ``dune_define_policy(<policy> <doc>)`` instead.

.. cmake:variable:: DUNE_POLICY_DEFAULT

  Default value of an unset policy. Supported values are ``OLD`` and ``NEW``.
  The default is ``OLD``.

.. cmake:variable:: DUNE_POLICY_DISABLE_WARNING

  If set to ``TRUE``, warnings about unset DUNE policies are suppressed. The
  default is ``FALSE``.

#]=======================================================================]
include_guard(GLOBAL)

set(DUNE_POLICY_DEFAULT "OLD" CACHE STRING "Default value for an unset dune policy.")
set_property(CACHE DUNE_POLICY_DEFAULT PROPERTY STRINGS "OLD" "NEW")
option(DUNE_POLICY_DISABLE_WARNING "Do not show warnings about unset dune policies." FALSE)

# print a help message with the signature of the dune_policy function
function(dune_policy_help _errorlevel _msg)
  message(${_errorlevel} "${_msg}\n"
    "The function `dune_policy` has the following signatures:\n"
    "  dune_policy(GET <policy> <var>): extract the value of the given <policy> in the variable <var>.\n"
    "  dune_policy(SET <policy> <value> [QUIET]): change the given <policy> to <value>.\n"
    "  dune_policy(LIST): list all registered policies with their values.\n"
    "  dune_policy(HELP): show this help message.\n")
endfunction(dune_policy_help)

# get the value of a _policy, or the DUNE_POLICY_DEFAULT
function(dune_get_policy _policy _var)
  get_property(_policy_defined GLOBAL PROPERTY DUNE_POLICY_${_policy} DEFINED)
  if(NOT _policy_defined)
    dune_policy_help(FATAL_ERROR "Undefined policy ${_policy}.")
  endif()
  get_property(_policy_set GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy} SET)
  if(NOT _policy_set)
    get_property(_policy_doc GLOBAL PROPERTY DUNE_POLICY_${_policy} BRIEF_DOCS)
    get_property(_policy_warning GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy}_WARNING)
    if(NOT _policy_warning AND NOT DUNE_POLICY_DISABLE_WARNING)
      message(AUTHOR_WARNING "Policy ${_policy} is not set: ${_policy_doc} "
        "Use the dune_policy(SET) command to set the policy and suppress this warning. "
        "The default value ${DUNE_POLICY_DEFAULT} will be used in the meantime. "
        "Setting a policy to NEW often requires some adaption to the buildsystem. "
        "See the buildsystem documentation for the details regarding the specific policy.")
      set_property(GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy}_WARNING TRUE)
    endif()
    set(_policy_value ${DUNE_POLICY_DEFAULT})
  else()
    get_property(_policy_value GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy})
  endif()
  set(${_var} ${_policy_value} PARENT_SCOPE)
endfunction(dune_get_policy)

# set a given _policy to the _value. If QUIET is passed as additional argument, ignore undefined policies.
function(dune_set_policy _policy _value)
  set(_quiet FALSE)
  if(ARGC GREATER 2 AND "${ARGV2}" STREQUAL "QUIET")
    set(_quiet TRUE)
  endif()
  get_property(_policy_defined GLOBAL PROPERTY DUNE_POLICY_${_policy} DEFINED)
  if(NOT _policy_defined)
    if(NOT _quiet)
      dune_policy_help(AUTHOR_WARNING "Undefined policy ${_policy} (ignored).")
    endif()
    return()
  endif()
  set_property(GLOBAL PROPERTY ${PROJECT_NAME}_POLICY_${_policy} ${_value})
endfunction(dune_set_policy)

# generic policy method to get, set, or list policies
function(dune_policy _method)
  if(_method STREQUAL "GET")
    if(ARGC LESS 3)
      dune_policy_help(FATAL_ERROR "Not enough arguments passed to dune_policy(GET...)")
    endif()
    dune_get_policy(${ARGV1} _var)
    set(${ARGV2} ${_var} PARENT_SCOPE)
  elseif(_method STREQUAL "SET")
    if(ARGC LESS 3)
      dune_policy_help(FATAL_ERROR "Not enough arguments passed to dune_policy(SET...)")
    endif()
    dune_set_policy(${ARGN})
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


function(dune_define_policy_impl _policy _doc)
  define_property(GLOBAL PROPERTY DUNE_POLICY_${_policy}
    BRIEF_DOCS "${_doc}" FULL_DOCS "${_doc}")
  set_property(GLOBAL APPEND PROPERTY DUNE_POLICIES ${_policy})
endfunction(dune_define_policy_impl)

# introduce a new policy into the system
function(dune_define_policy)
  if(ARGC EQUAL 2)
    dune_define_policy_impl(${ARGV0} ${ARGV1})
  elseif(ARGC EQUAL 4)
    # fallback for old implementation with 4 arguments.
    # ignore the <module> and the <version> argument.
    message(DEPRECATION "The signature dune_define_policy(<policy> <module> <version> <doc>) is deprecated. Use dune_define_policy(<policy> <doc>) instead. The <module> and <version> parameter are ignored. The old interface will be removed after the dune-common 2.12 release.")
    dune_define_policy_impl(${ARGV0} ${ARGV3})
  else()
    message(FATAL_ERROR "Usage dune_define_policy(<policy> <doc>).")
  endif()
endfunction(dune_define_policy)
