dnl -*- mode: autoconf; tab-width: 8; indent-tabs-mode: nil; -*-
dnl vi: set et ts=8 sw=2 sts=2:
# searches for Threading Building Blocks headers and libs

AC_DEFUN([DUNE_PATH_TBB],[
  AC_MSG_NOTICE([checking for Threading Building Blocks library...])
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([ACX_PTHREAD])
  AC_REQUIRE([GXX0X])
  AC_REQUIRE([LAMBDA_EXPR_CHECK])
  AC_LANG_PUSH([C++])
  #
  # user hints
  #
  AC_ARG_VAR([TBBVARS_SH], [Location of Threading Building Blocks tbbvars.sh script])
  AC_ARG_ENABLE([tbb_allocator],
    [AS_HELP_STRING([--enable-tbb-allocator],
      [When using Threading Building Blocks, also link against the tbbmalloc library to enable the optional scalable memory allocator that is part of TBB @<:@default=yes@:>@])],
    [
      AS_CASE(["$enableval"],
        [yes], [tbb_allocator="yes"],
        [no ], [tbb_allocator="no" ],
               [AC_MSG_ERROR(["invalid value '$enableval' for --enable-tbb-allocator, must be 'yes' or 'no'"])
        ])
    ],
    [
      tbb_allocator="yes"
    ])
  AC_ARG_ENABLE([tbb_rpath],
    [AS_HELP_STRING([--enable-tbb-rpath],
      [When using Threading Building Blocks, embed the path to the libraries into all generated binaries instead of relying a correctly set LD_LIBRARY_PATH. Warning, this does not work on every system due to differences in the linker syntax! @<:@default=no@:>@])],
    [
      AS_CASE(["$enableval"],
        [yes], [tbb_rpath="yes"],
        [no ], [tbb_rpath="no" ],
               [AC_MSG_ERROR(["invalid value '$enableval' for --enable-tbb-rpath, must be 'yes' or 'no'"])
        ])
    ],
    [
      tbb_rpath="no"
    ])
  AC_ARG_WITH([tbb],
    [AS_HELP_STRING([--with-tbb=<tbbvars.sh>],
      [Enable support for the Threading Building Blocks library. If TBB is not in the default paths, you need to pass the full path to the tbbvars.sh script inside the precise library directory you want to use (release/debug). Note that this library is always linked dynamically, so make sure to either update your LD_LIBRARY_PATH or use "--enable-tbb-rpath". When the provided tbbvars.sh script points to a debug or a community preview feature version of TBB, configure will try to detect this and set the necessary preprocessor flags. @<:@default=check@:>@])],
    [
      tbbvars="$withval"
    ],
    [
      # grab value from environment variable if available
      AS_IF([ test -n "$TBBVARS_SH" ],
        [ tbbvars="$TBBVARS_SH" ],
        [ tbbvars="check" ])
    ])

  AC_MSG_CHECKING([for TBB installation])

  AS_IF([ test "x$tbbvars" = "xno" ],
    [
      AC_MSG_RESULT([disabled by user])
      with_tbb="no"
    ],
    [ test "x$tbbvars" = "xcheck" ],
    [
      # look for TBB somewhere in the system standard paths
      AC_MSG_RESULT([default])
      AS_IF([ test "$tbb_rpath" = "yes" ],
        [ AC_MSG_WARN([TBB Library path embedding into rpath only works when specifying tbbvars.sh]) ])
    ],
    [ test "x$tbbvars" = "xyes" ],
    [
      # look for TBB somewhere in the system standard paths
      AC_MSG_RESULT([default])
      AS_IF([ test "x$tbb_rpath" = "xyes" ],
        [ AC_MSG_WARN([TBB Library path embedding into rpath only works when specifying tbbvars.sh]) ])
      # user wants TBB, make sure to fail if we can't find it
      tbb_required="yes"
    ],
    [
      # user wants TBB, make sure to fail if we can't find it
      tbb_required="yes"
      # user has provided a tbbvars.sh script, extract information
      AS_IF([ test ! -f "$tbbvars" ],
        [
          # could not find file, abort
          AC_MSG_RESULT([failed])
          AC_MSG_WARN([tbbvars.sh script "$tbbvars" not found])
          with_tbb="no"
        ],
        [
          # determine architecture for tbbvars
          AS_CASE([-$host-],
          [*-k1om-*], [
            tbb_host_arch=k1om
          ],
          [*-i?86-*|*-ia32-*], [
            tbb_host_arch=ia32
          ],
          [*-x86_64-*], [
            tbb_host_arch=intel64
          ],
          [
            tbb_host_arch=
          ])
          # setup include and library paths from tbbvars.sh
          AS_CASE([$tbb_host_arch],
          [""], [
            tbb_lib_dir=
            tbb_root_dir=
          ],
          [k1om], [
            tbb_lib_dir=$(
              MIC_LD_LIBRARY_PATH=
              set -- intel64
              source "$tbbvars" >/dev/null 2>&1
              echo "$MIC_LD_LIBRARY_PATH"
            )
            tbb_root_dir=$(
              set -- intel64
              source "$tbbvars" >/dev/null 2>&1
              echo "$TBBROOT"
            )
          ],
          [
            tbb_lib_dir=$(
              LD_LIBRARY_PATH=
              set -- "$tbb_host_arch"
              source "$tbbvars" >/dev/null 2>&1
              echo "$LD_LIBRARY_PATH"
            )
            tbb_root_dir=$(
              set -- "$tbb_host_arch"
              source "$tbbvars" >/dev/null 2>&1
              echo "$TBBROOT"
            )
          ])
          AS_IF(
            [ test -z "$tbb_host_arch" ], [
              AC_MSG_RESULT([failed])
              AC_MSG_NOTICE([I don't know how to invoke $tbbvars for host=$host])
            ],
            [ test ! -d "$tbb_root_dir" ], [
              # tbbvars.sh contained bogus information, abort
              AC_MSG_RESULT([failed])
              AC_MSG_WARN([invalid TBB installation root directory "$tbb_root_dir"])
              with_tbb="no"
            ],
            [
              # convert paths to absolute paths
              tbb_lib_dir="$(cd -- "$tbb_lib_dir" ; echo $PWD)"
              tbb_root_dir="$(cd -- "$tbb_root_dir" ; echo $PWD)"
              TBB_CPPFLAGS="-I$tbb_root_dir/include ${PTHREAD_CFLAGS}"
              TBB_LDFLAGS="-L$tbb_lib_dir"
              TBB_LIBS=$PTHREAD_LIBS
              AC_MSG_RESULT([from "$tbbvars"])
              AC_MSG_NOTICE([  TBB include directory "$tbb_root_dir/include"])
              AC_MSG_NOTICE([  TBB library directory "$tbb_lib_dir"])
              AS_IF([ test "x$tbb_rpath" = "xyes" ],
                [
                  AC_MSG_NOTICE([    Encoding TBB library directory into binary rpath])
                  TBB_LDFLAGS="$TBB_LDFLAGS -Wl,-rpath,$tbb_lib_dir"
                ])
            ])
        ])
    ])

  # save compiler variables
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"

  AS_IF([ test "x$with_tbb" != "xno" ],
    [
      # check for TBB headers
      CPPFLAGS="$CPPFLAGS $TBB_CPPFLAGS"
      AC_CHECK_HEADER([tbb/tbb_stddef.h],[],[],[])
      AS_IF([ test "x$ac_cv_header_tbb_tbb_stddef_h" = "xno" ],
        [ with_tbb="no" ])
    ])

  AS_IF([ test "x$with_tbb" != "xno" ],
    [
      # try to find a valid library
      LDFLAGS="$LDFLAGS $TBB_LDFLAGS"
      LIBS="$TBB_LIBS $LIBS"
      AC_SEARCH_LIBS([TBB_runtime_interface_version],[tbb tbb_debug tbb_preview tbb_preview_debug])
      AS_IF([ test "x$ac_cv_search_TBB_runtime_interface_version" = "xno" ],
        [ with_tbb="no" ])
    ])

  AS_IF([ test "x$with_tbb" != "xno" ],
    [
      tbb_lib="$ac_cv_search_TBB_runtime_interface_version"

      # check for debug and / or preview build
      AS_IF([ test "x$(echo $tbb_lib | grep -o debug)" = "xdebug" ],
        [
          tbb_is_debug="yes"
          TBB_CPPFLAGS="$TBB_CPPFLAGS -DTBB_USE_DEBUG"
          CPPFLAGS="$CPPFLAGS -DTBB_USE_DEBUG"
        ],
        [
          tbb_is_debug="no"
        ])

      AS_IF([ test "x$(echo $tbb_lib | grep -o preview)" = "xpreview" ],
        [
          tbb_is_preview="yes"
          TBB_CPPFLAGS="$TBB_CPPFLAGS -DTBB_PREVIEW_TASK_ARENA -DTBB_PREVIEW_LOCAL_OBSERVER"
          CPPFLAGS="$CPPFLAGS -DTBB_PREVIEW_TASK_ARENA -DTBB_PREVIEW_LOCAL_OBSERVER"
        ],
        [
          tbb_is_preview="no"
        ])

      TBB_LIBS="$tbb_lib $TBB_LIBS"

      AS_IF([ test "x$tbb_allocator" = "xyes" ],
        [
          # add tbbmalloc library
          AC_MSG_NOTICE([Adding TBB allocator library...])
          AS_IF([ test "x$tbb_is_debug" = "xyes" ],
            [ TBB_LIBS="$TBB_LIBS -ltbbmalloc_debug" ],
            [ TBB_LIBS="$TBB_LIBS -ltbbmalloc" ])
        ])

      LIBS="$TBB_LIBS $ac_save_LIBS"

      # perform final test to check whether everything really works
      AC_MSG_CHECKING([for working TBB])

      AC_LINK_IFELSE([
        AC_LANG_PROGRAM([[
                          #include <tbb/tbb.h>
                          #include <numeric>

                          #ifndef HAVE_LAMBDA_EXPR
                          // provide a fallback for compilers without lambdas
                          struct add
                          {
                            add(int* x)
                              : _x(x)
                            {}

                            int* _x;

                            void operator()(int i) const
                            {
                              _x[i] = i;
                            }
                          };
                          #endif

                        ]],
                        [[
                            int x[10] = {0};
                          #ifdef HAVE_LAMBDA_EXPR
                            tbb::parallel_for(0,10,[&](int i){ x[i] = i; });
                          #else
                            tbb::parallel_for(0,10,add(x));
                          #endif
                            return !(std::accumulate(x,x+10,0) == (9*10)/2);
                        ]])],
        [
          with_tbb="yes"
          AC_MSG_RESULT([yes])
        ],
        [
          AC_MSG_RESULT([failed])
          with_tbb="no"
        ])
    ])

  # tests are done, set up output

  AS_IF([ test "x$with_tbb" = "xyes" ],
    [
      HAVE_TBB=1

      TBB_CPPFLAGS="$TBB_CPPFLAGS -DENABLE_TBB"

      AC_SUBST(TBB_CPPFLAGS,$TBB_CPPFLAGS)
      AC_SUBST(TBB_LDFLAGS,$TBB_LDFLAGS)
      AC_SUBST(TBB_LIBS,$TBB_LIBS)

      # define main preprocessor macro
      AC_DEFINE(HAVE_TBB,ENABLE_TBB,[Define if you have the Threading Building Blocks library.
                                     This is only true if the library was found _and_ if the
                                     application uses the TBB_CPPFLAGS])

      # additional preprocessor macro for allocator
      AS_IF([ test "x$tbb_allocator" = "xyes" ],
        [
          AC_DEFINE(HAVE_TBB_ALLOCATOR,ENABLE_TBB,[Define if you have the optional allocator library from
                                                   Threading Building Blocks.
                                                   This is only true if the library was found _and_ if the
                                                   application uses the TBB_CPPFLAGS])
        ])

      # additional preprocessor macro for community features
      AS_IF([ test "x$tbb_is_preview" = "xyes" ],
        [
          AC_DEFINE(HAVE_TBB_PREVIEW,ENABLE_TBB,[Define if the community preview features of the
                                                 Threading Building Blocks library are available.
                                                 This is only true if the features are enabled _and_ if the
                                                 application uses the TBB_CPPFLAGS])
        ])

      # add to global list
      DUNE_ADD_ALL_PKG([TBB],[\${TBB_CPPFLAGS}],[\${TBB_LDFLAGS}],[\${TBB_LIBS}])

      # create summary string
      AS_IF([ test "x$tbb_is_debug" = "xyes" ],
        [ tbb_summary=debug ],
        [ tbb_summary=release ])

      AS_IF([ test "x$tbb_allocator" = "xyes" ],
        [ tbb_summary="$tbb_summary,allocator" ])

      AS_IF([ test "x$tbb_is_preview" = "xyes" ],
        [ tbb_summary="$tbb_summary,community preview" ])

      AS_IF([ test "x$tbb_rpath" = "xyes" ],
        [ tbb_summary="$tbb_summary,rpath encoded" ])

      tbb_summary="yes ($tbb_summary)"

    ],
    [ test "x$tbb_required" = "xyes" ],
    [
      # bail out
      AC_MSG_FAILURE([Could not find a working version of Threading Building Blocks])
    ],
    [
      # TBB is optional, just log that it is missing
      tbb_summary="no"
    ])

  # restore compiler variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"

  AC_LANG_POP

  # tell automake
  AM_CONDITIONAL(TBB, test x$HAVE_TBB = x1)

  DUNE_ADD_SUMMARY_ENTRY([TBB],["$tbb_summary"])

])
