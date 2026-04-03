# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneStreams
===========

Configuration helpers for the DUNE debug streams.

.. cmake:command:: dune_set_minimal_debug_level

  Initialize the cache variables controlling the built-in DUNE debug streams.

  .. code-block:: cmake

    dune_set_minimal_debug_level()

  The command maps the textual cache value in
  :cmake:variable:`MINIMAL_DEBUG_LEVEL` to the numeric preprocessor value
  ``DUNE_MINIMAL_DEBUG_LEVEL`` used by the C++ debug stream implementation.

.. cmake:variable:: MINIMAL_DEBUG_LEVEL

  Minimal enabled DUNE debug stream level. Streams below this level collapse
  to no-ops when output is requested.

  Supported values are ``vverb``, ``verb``, ``info``, ``warn``, and
  ``grave``. The default is ``warn``.

#]=======================================================================]
include_guard(GLOBAL)

macro(dune_set_minimal_debug_level)
set(MINIMAL_DEBUG_LEVEL "warn" CACHE STRING "set the MINIMAL_DEBUG_LEVEL. Standard debug streams with level below MINIMAL_DEBUG_LEVEL will collapse to doing nothing if output is requested. (default=warn)")
set_property(CACHE MINIMAL_DEBUG_LEVEL PROPERTY STRINGS
  "grave" "warn" "info" "verb" "vverb")
if(MINIMAL_DEBUG_LEVEL STREQUAL "grave")
  set(DUNE_MINIMAL_DEBUG_LEVEL 5)
elseif(MINIMAL_DEBUG_LEVEL STREQUAL "info")
  set(DUNE_MINIMAL_DEBUG_LEVEL 3)
elseif(MINIMAL_DEBUG_LEVEL STREQUAL "verb")
  set(DUNE_MINIMAL_DEBUG_LEVEL 2)
elseif(MINIMAL_DEBUG_LEVEL STREQUAL "vverb")
  set(DUNE_MINIMAL_DEBUG_LEVEL 1)
# default to warn
else()
  set(DUNE_MINIMAL_DEBUG_LEVEL 4)
endif()
message(STATUS "Setting the Dune debug stream minimal verbosity level to ${DUNE_MINIMAL_DEBUG_LEVEL} (${MINIMAL_DEBUG_LEVEL})")
endmacro(dune_set_minimal_debug_level)
