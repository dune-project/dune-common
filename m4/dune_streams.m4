# $Id: dune_deprecated.m4 1114 2004-11-17 14:00:06Z christi $

# Set the minimal verbosity level. All stream with a level below will
# get removed by the optimizer.

AC_DEFUN([DUNE_SET_MINIMAL_DEBUG_LEVEL],[

  # VERY_VERBOSE_DEBUG_LEVEL = 1;
  # VERBOSE_DEBUG_LEVEL = 2;
  # INFO_DEBUG_LEVEL = 3;
  # WARN_DEBUG_LEVEL = 4;
  # GRAVE_DEBUG_LEVEL = 5;

  AC_ARG_WITH(minimal_debug_level,
    AS_HELP_STRING([--with-minimal-debug-level=[grave|warn|info|verb|vverb]],
     [set the MINIMAL_DEBUG_LEVEL. Standard debug streams with level below 
      MINIMAL_DEBUG_LEVEL will collapse to doing nothing if output is 
      requested. (default=warn)]
    )
  )

  MINIMAL_DEBUG_LEVEL=4

  case "$with_minimal_debug_level" in
    grave)
      MINIMAL_DEBUG_LEVEL=5
    ;;
    warn)
      MINIMAL_DEBUG_LEVEL=4
    ;;
    info)
      MINIMAL_DEBUG_LEVEL=3
    ;;
    verb)
      MINIMAL_DEBUG_LEVEL=2
    ;;
    vverb)
      MINIMAL_DEBUG_LEVEL=1
    ;;
  esac

  AC_DEFINE_UNQUOTED([DUNE_MINIMAL_DEBUG_LEVEL], $MINIMAL_DEBUG_LEVEL,
    [Standard debug streams with a level below will collapse to doing nothing])
])
