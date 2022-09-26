# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# This Module configures the DUNE debug streams.
#
# .. cmake_variable:: MINIMAL_DEBUG_LEVEL
#
#    This variable configures the Dune debug streams.
#    Standard debug streams with level below :code:`MINIMAL_DEBUG_LEVEL` will
#    collapse to doing nothing if output is requested. Possible values are
#    :code:`vverb`, :code:`verb`, :code:`info`, :code:`warn` and :code:`grave`.
#    Defaults to :code:`warn`.
#
include_guard(GLOBAL)

macro(dune_set_minimal_debug_level)
set(MINIMAL_DEBUG_LEVEL ON CACHE STRING "set the MINIMAL_DEBUG_LEVEL. Standard debug streams with level below MINIMAL_DEBUG_LEVEL will collapse to doing nothing if output is requested. (default=warn)")
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
message(STATUS "Set Minimal Debug Level to ${DUNE_MINIMAL_DEBUG_LEVEL}")
endmacro(dune_set_minimal_debug_level)
