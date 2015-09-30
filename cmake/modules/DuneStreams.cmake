# This Module configures the DUNE debug streams.
#
# .. cmake_variable:: MINIMAL_DEBUG_LEVEL
#
#    This variable configures the Dune debug streams.
#    Standard debug streams with level below :code:`MINIMAL_DEBUG_LEVEL` will
#    collapse to doing nothing if output is requested. Possible values are
#    :code:`vverb`, :code:`verb`, :code:`info`, :code:`warn` and :code:`grave`.
#    Deafults to :code:`warn`.
#

macro(dune_set_minimal_debug_level)
set(MINIMAL_DEBUG_LEVEL ON CACHE STRING "set the MINIMAL_DEBUG_LEVEL. Standard debug streams with level below MINIMAL_DEBUG_LEVEL will collapse to doing nothing if output is requested. (default=warn)")
set_property(CACHE MINIMAL_DEBUG_LEVEL PROPERTY STRINGS
  "grave" "warn" "info" "verb" "vverb")
if(MINIMAL_DEBUG_LEVEL MATCHES "grave")
  set(DUNE_MINIMAL_DEBUG_LEVEL 5)
elseif(MINIMAL_DEBUG_LEVEL MATCHES "info")
  set(DUNE_MINIMAL_DEBUG_LEVEL 3)
elseif(MINIMAL_DEBUG_LEVEL MATCHES "verb")
  set(DUNE_MINIMAL_DEBUG_LEVEL 2)
elseif(MINIMAL_DEBUG_LEVEL MATCHES "vverb")
  set(DUNE_MINIMAL_DEBUG_LEVEL 1)
else()
  set(DUNE_MINIMAL_DEBUG_LEVEL 4)
endif()
endmacro(dune_set_minimal_debug_level)
