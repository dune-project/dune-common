dnl -*- autoconf -*-
# $Id$
# checks for dune-headers and everything they need

# TODO
#
# - use pkg-config if --enable-localdune is not provided

#   #export PKG_CONFIG_LIBDIR=$with_dune/dune
#  #PKG_CHECK_MODULES(DUNE, dune)  

AC_DEFUN([DUNE_DISABLE_LIBCHECK],[
  # hidden feature... --enable-dunelibcheck tells the dune module check to
  # check only for the headers and not for the libs. This feature is used
  # when building the web page, because we don't need the libs in this case

  AC_ARG_ENABLE([dunelibcheck],[])
])

AC_DEFUN([DUNE_MODULE_ADD_SUMMARY_ENTRY],[
  m4_pushdef([_dune_name], [$1])
  m4_pushdef([_dune_module], [m4_translit(_dune_name, [-], [_])])
  m4_pushdef([_DUNE_MODULE], [m4_toupper(_dune_module)])
  result="$with_[]_dune_module"
  AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
    result="$result ($_DUNE_MODULE[]_ROOT)"
  ])
  AS_IF([test -n "$_DUNE_MODULE[]_VERSION"],[
    result="$result version $_DUNE_MODULE[]_VERSION"
  ])
  DUNE_ADD_SUMMARY_MOD_ENTRY(_dune_name,[$result])
])

m4_define([_dune_sub_version],"`echo $1 | $AWK -F. -v FIELD=$2 '{ print int($FIELD) }'`")

# DUNE_PARSE_MODULE_VERSION(NAME, VERSION)
#
# NAME    Name of the module, lowercase with dashes (like "dune-common").  The
#         value must be known when autoconf runs, so shell variables in the
#         value are not permissible.
# VERSION Version of the module.  May contain shell variables.  Numbers
#         seperated by ".".
#
# In the following, {MODULE} is the uppercase version of {NAME} with any "-"
# replaced by "_".
#
# configure/shell variables and preprocessor defines:
#   {MODULE}_VERSION (complete version, same as {VERSION})
#   {MODULE}_VERSION_MAJOR (first component of {VERSION})
#   {MODULE}_VERSION_MINOR (second component of {VERSION})
#   {MODULE}_VERSION_REVISION (third component of {VERSION})
AC_DEFUN([DUNE_PARSE_MODULE_VERSION],[
  AC_REQUIRE([AC_PROG_AWK])

  m4_pushdef([_dune_name], [$1])
  m4_pushdef([_dune_version], [$2])
  m4_pushdef([_dune_module], [m4_translit(_dune_name, [-], [_])])
  m4_pushdef([_DUNE_MODULE], [m4_toupper(_dune_module)])

  _DUNE_MODULE[]_VERSION=_dune_version
  _DUNE_MODULE[]_VERSION_MAJOR=_dune_sub_version(_dune_version,1)
  _DUNE_MODULE[]_VERSION_MINOR=_dune_sub_version(_dune_version,2)
  _DUNE_MODULE[]_VERSION_REVISION=_dune_sub_version(_dune_version,3)

  AC_DEFINE_UNQUOTED(_DUNE_MODULE[]_VERSION, "_dune_version", [Define to the version of] _dune_name)
  AC_DEFINE_UNQUOTED(_DUNE_MODULE[]_VERSION_MAJOR, $_DUNE_MODULE[]_VERSION_MAJOR, [Define to the major version of] _dune_name)
  AC_DEFINE_UNQUOTED(_DUNE_MODULE[]_VERSION_MINOR, $_DUNE_MODULE[]_VERSION_MINOR, [Define to the minor version of] _dune_name)
  AC_DEFINE_UNQUOTED(_DUNE_MODULE[]_VERSION_REVISION, $_DUNE_MODULE[]_VERSION_REVISION, [Define to the revision of] _dune_name)

  m4_popdef([_DUNE_MODULE])
  m4_popdef([_dune_module])
  m4_popdef([_dune_version])
  m4_popdef([_dune_name])
])

# DUNE_CHECK_MODULES(NAME, HEADER, SYMBOL)
#
# Generic check for dune modules.  This macro should not be used directly, but
# in the modules m4/{module}.m4 in the {MODULE}_CHECK_MODULE macro.  The
# {MODULE}_CHECK_MODULE macro knows the parameters to call this
# DUNE_CHECK_MODULES macro with, and it does not take any parameters itself,
# so it may be used with AC_REQUIRE.
#
# NAME   Name of the module, lowercase with dashes (like "dune-common").  The
#        value must be known when autoconf runs, so shell variables in the
#        value are not permissible.
#
# HEADER Header to check for.  The check will really be for <dune/{HEADER}>,
#        so the header must reside within a directory called "dune".
#
# SYMBOL Symbol to check for in the module's library.  If this argument is
#        empty or missing, it is assumed that the module does not provide a
#        library.  The value must be known when autoconf runs, so shell
#        variables in the value are not permissible.  This value is actually
#        handed to AC_TRY_LINK unchanged as the FUNCTION-BODY argument, so it
#        may contain more complex stuff than a simple symbol.
#
#        The name of the library is assumed to be the same as the module name,
#        with any occurance of "-" removed.  The path of the library is
#        obtained from pkgconfig for installed modules, or assumed to be the
#        directory "lib" within the modules root for non-installed modules.
#
# In the following, {module} is {NAME} with any "-" replaced by "_" and
# {MODULE} is the uppercase version of {module}.
#
# configure options:
#   --with-{NAME}
#
# configure/shell variables:
#   {MODULE}_CPPFLAGS
#   {MODULE}_ROOT
#   {MODULE}_LDFLAGS
#   {MODULE}_LIBS
#   HAVE_{MODULE} (1 or 0)
#   with_{module} ("yes" or "no")
#   DUNE_CPPFLAGS (adds the modules values here)
#   DUNE_LDFLAGS (adds the modules values here)
#   DUNE_LIBS (adds the modules values here)
#   DUNE_PKG_CPPFLAGS (deprecated, adds the modules values here)
#   DUNE_PKG_LDFLAGS (deprecated, adds the modules values here)
#   DUNE_PKG_LIBS (deprecated, adds the modules values here)
#   {MODULE}_VERSION
#   {MODULE}_VERSION_MAJOR
#   {MODULE}_VERSION_MINOR
#   {MODULE}_VERSION_REVISION
#
# configure substitutions/makefile variables:
#   {MODULE}_CPPFLAGS
#   {MODULE}_LDFLAGS
#   {MODULE}_LIBS
#   {MODULE}_ROOT
#   DUNE_CPPFLAGS
#   DUNE_LDFLAGS
#   DUNE_LIBS
#
# preprocessor defines:
#   HAVE_{MODULE} (1 or undefined)
#   {MODULE}_VERSION
#   {MODULE}_VERSION_MAJOR
#   {MODULE}_VERSION_MINOR
#   {MODULE}_VERSION_REVISION
#
# automake conditionals:
#   HAVE_{MODULE}
AC_DEFUN([DUNE_CHECK_MODULES],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])
  AC_REQUIRE([DUNE_DISABLE_LIBCHECK])
  AC_REQUIRE([LT_OUTPUT])

  # ____DUNE_CHECK_MODULES_____ ($1)

  m4_pushdef([_dune_name], [$1])
  m4_pushdef([_dune_module], [m4_translit(_dune_name, [-], [_])])
  m4_pushdef([_dune_header], [$2])
  m4_pushdef([_dune_ldpath], [lib])
  m4_pushdef([_dune_lib],    [m4_translit(_dune_name, [-], [])])
  m4_pushdef([_dune_symbol], [$3])
  m4_pushdef([_DUNE_MODULE], [m4_toupper(_dune_module)])

  # switch tests to c++
  AC_LANG_PUSH([C++])

  # the usual option...
  AC_ARG_WITH(_dune_name,
    AS_HELP_STRING([--with-_dune_name=PATH],[_dune_module directory]))

  # backup of flags
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  ac_save_LDFLAGS="$LDFLAGS"
  CPPFLAGS=""
  LIBS=""

  ##
  ## Where is the module $1?
  ##

  AC_MSG_CHECKING([for $1 installation or source tree])

  # is a directory set?
  AS_IF([test -z "$with_[]_dune_module"],[
    #
    # search module $1 via pkg-config
    #
    with_[]_dune_module="global installation"
    AS_IF([test -z "$PKG_CONFIG"],[
      AC_MSG_RESULT([failed])
      AC_MSG_NOTICE([could not search for module _dune_name])
      AC_MSG_ERROR([pkg-config is required for using installed modules])
    ])
    AS_IF(AC_RUN_LOG([$PKG_CONFIG --exists --print-errors "$1"]),[
      _DUNE_MODULE[]_CPPFLAGS="`$PKG_CONFIG --cflags _dune_name`" 2>/dev/null
      _DUNE_MODULE[]_ROOT="`$PKG_CONFIG --variable=prefix _dune_name`" 2>/dev/null 
      _DUNE_MODULE[]_VERSION="`$PKG_CONFIG --modversion _dune_name`" 2>/dev/null
      _DUNE_MODULE[]_LDFLAGS=""
      ifelse(_dune_symbol,,
        [_DUNE_MODULE[]_LIBS=""],
        [_DUNE_MODULE[]_LIBS="-L`$PKG_CONFIG --variable=libdir _dune_name 2>/dev/null` -l[]_dune_lib"])
      HAVE_[]_DUNE_MODULE=1
      AC_MSG_RESULT([global installation in $_DUNE_MODULE[]_ROOT])
    ],[
      HAVE_[]_DUNE_MODULE=0
      AC_MSG_RESULT([not found])
    ])
  ],[
    #
    # path for module $1 is specified via command line
    #
    AS_IF([test -d "$with_[]_dune_module"],[
      # expand tilde / other stuff
      _DUNE_MODULE[]_ROOT=`cd $with_[]_dune_module && pwd`

      # expand search path (otherwise empty CPPFLAGS)
      AS_IF([test -d "$_DUNE_MODULE[]_ROOT/include/dune"],[
        # Dune was installed into directory given by with-dunecommon
        _DUNE_MODULE[]_CPPFLAGS="-I$_DUNE_MODULE[]_ROOT/include"
        _DUNE_MODULE[]_BUILDDIR=_DUNE_MODULE[]_ROOT
        _DUNE_MODULE[]_VERSION="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$_DUNE_MODULE[]_ROOT/lib/pkgconfig $PKG_CONFIG --modversion _dune_name`" 2>/dev/null
      ],[
        _DUNE_MODULE[]_SRCDIR=$_DUNE_MODULE[]_ROOT
        # extract src and build path from Makefile, if found
	    AS_IF([test -f $_DUNE_MODULE[]_ROOT/Makefile],[
          _DUNE_MODULE[]_SRCDIR="`sed -ne '/^abs_top_srcdir = /{s/^abs_top_srcdir = //; p}' $_DUNE_MODULE[]_ROOT/Makefile`"
		])
        _DUNE_MODULE[]_CPPFLAGS="-I$_DUNE_MODULE[]_SRCDIR"
        _DUNE_MODULE[]_VERSION="`grep Version $_DUNE_MODULE[]_SRCDIR/dune.module | sed -e 's/^Version: *//'`" 2>/dev/null
      ])
      _DUNE_MODULE[]_LDFLAGS=""
      ifelse(_dune_symbol,,
        [_DUNE_MODULE[]_LIBS=""],
        [_DUNE_MODULE[]_LIBS="-L$_DUNE_MODULE[]_ROOT/lib -l[]_dune_lib"])
      # set expanded module path
      with_[]_dune_module="$_DUNE_MODULE[]_ROOT"
      HAVE_[]_DUNE_MODULE=1
      AC_MSG_RESULT([found in $_DUNE_MODULE[]_ROOT])
    ],[
      HAVE_[]_DUNE_MODULE=0
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([_dune_name-directory $with_[]_dune_module does not exist])
    ])
  ])

  DUNE_CPPFLAGS="$DUNE_CPPFLAGS $_DUNE_MODULE[]_CPPFLAGS"
  CPPFLAGS="$DUNE_CPPFLAGS"
  SET_CPPFLAGS="$_DUNE_MODULE[]_CPPFLAGS"

  ##  
  ## check for an arbitrary header
  ##
  AC_CHECK_HEADER([dune/[]_dune_header],
    [HAVE_[]_DUNE_MODULE=1
     _DUNE_MODULE[]_CPPFLAGS="$SET_CPPFLAGS"],
    [HAVE_[]_DUNE_MODULE=0
     _DUNE_MODULE[]_CPPFLAGS=""
     AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
       AC_MSG_WARN([$_DUNE_MODULE[]_ROOT does not seem to contain a valid _dune_name (dune/[]_dune_header not found)])
     ])
    ]
  )

  ##
  ## check for lib (if lib name was provided)
  ##
  ifelse(_dune_symbol,,
    AC_MSG_NOTICE([_dune_name does not provide libs]),

    AS_IF([test "x$enable_dunelibcheck" = "xno"],[
      AC_MSG_WARN([library check for _dune_name is disabled. DANGEROUS!])
    ],[
      AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[

        # save current LDFLAGS
        ac_save_LDFLAGS="$LDFLAGS"

        ac_save_LIBS="$LIBS"
        ac_save_CXX="$CXX"
        HAVE_[]_DUNE_MODULE=0

        # define LTCXXCOMPILE like it will be defined in the Makefile
        LTCXXLINK="./libtool --tag=CXX --mode=link $CXX $CXXFLAGS $LDFLAGS"
        CXX="$LTCXXLINK"

        # use module LDFLAGS
        LDFLAGS="$LDFLAGS $DUNE_PKG_LDFLAGS $_DUNE_MODULE[]_LDFLAGS"
        LIBS="$DUNE_LIBS $_DUNE_MODULE[]_LIBS"

        AC_MSG_CHECKING([for lib[]_dune_lib])

        AC_TRY_LINK(dnl
          [#]include<dune/[]_dune_header>,
          _dune_symbol,
          [
            AC_MSG_RESULT([yes])
            HAVE_[]_DUNE_MODULE=1
            _DUNE_MODULE[]_LIBS="$LIBS"
          ],[
            AC_MSG_RESULT([no])
            HAVE_[]_DUNE_MODULE=0
            AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
             AC_MSG_WARN([$with_[]_dune_module does not seem to contain a valid _dune_name (failed to link with lib[]_dune_lib[].la)])
            ])
          ]
        )
      ])

      # reset variables
      LDFLAGS="$ac_save_LDFLAGS"
      LIBS="$ac_save_LIBS"
      CXX="$ac_save_CXX"
    ])
  )

  # did we succeed?
  AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[
    # set variables for our modules
    AC_SUBST(_DUNE_MODULE[]_CPPFLAGS, "$_DUNE_MODULE[]_CPPFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LDFLAGS, "$_DUNE_MODULE[]_LDFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LIBS, "$_DUNE_MODULE[]_LIBS")
    AC_SUBST(_DUNE_MODULE[]_ROOT, "$_DUNE_MODULE[]_ROOT")
    AC_DEFINE(HAVE_[]_DUNE_MODULE, 1, [Define to 1 if] _dune_name [was found])

    DUNE_PARSE_MODULE_VERSION(_dune_name, $_DUNE_MODULE[]_VERSION)

    # set DUNE_* variables
    AC_SUBST(DUNE_CPPFLAGS, "$DUNE_CPPFLAGS")
    AC_SUBST(DUNE_LDFLAGS, "$DUNE_LDFLAGS $_DUNE_MODULE[]_LDFLAGS")
    AC_SUBST(DUNE_LIBS, "$DUNE_LIBS $_DUNE_MODULE[]_LIBS")
    
    # add to global list
    # only add my flags other flags are added by other packages 
    DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $_DUNE_MODULE[]_CPPFLAGS"
    DUNE_PKG_LIBS="$DUNE_PKG_LIBS $LIBS"
    DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $_DUNE_MODULE[]_LDFLAGS"

    with_[]_dune_module="yes"
  ],[
    with_[]_dune_module="no"
  ])

  AM_CONDITIONAL(HAVE_[]_DUNE_MODULE, test x$HAVE_[]_DUNE_MODULE = x1)

  # reset previous flags
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"

  # add this module to DUNE_SUMMARY
  DUNE_MODULE_ADD_SUMMARY_ENTRY(_dune_name)

  # remove local variables
  m4_popdef([_dune_name])
  m4_popdef([_dune_module])
  m4_popdef([_dune_header])
  m4_popdef([_dune_ldpath])
  m4_popdef([_dune_lib])
  m4_popdef([_dune_symbol])
  m4_popdef([_DUNE_MODULE])

  # restore previous language settings (leave C++)
  AC_LANG_POP([C++])
])

AC_DEFUN([DUNE_DEV_MODE],[
  AC_ARG_ENABLE(dunedevel,
    AS_HELP_STRING([--enable-dunedevel],[activate Dune-Developer-mode]))

  AS_IF([test "x$enable_dunedevel" = "xyes"],
    AC_DEFINE(DUNE_DEVEL_MODE, 1, [Activates developer output]))
])

AC_DEFUN([DUNE_SYMLINK],[
  AC_PROG_LN_S
  AC_MSG_WARN([Module is using the deprecated 'dune' symlink.])
  # create symlink for consistent paths even when $(top_srcdir) is not
  # called dune/ (if filesystem/OS supports symlinks)
  AS_IF([test "x$LN_S" = "xln -s"],[
    # Symlinks possible!

    # Note: we are currently in the build directory which may be != the
    # source directory
 
    # does a file already exist?
    AS_IF([test -e "dune"],[
      # exists: is a symlink?
      AS_IF([test -L "dune"],[
        AS_IF([! test -r dune/$ac_unique_file],[
          AC_MSG_ERROR([Symlink 'dune' exists but appears to be wrong! Please remove it manually])
        ])
      ],[
        # if we are in the source directory we can make sure that there is no directory
        AC_MSG_ERROR([Module is using the DUNE[]_SYMLINK directive but contains a directory 'dune'!])
      ])
    ],[
      echo Creating dune-symlink...
      # set symlink in build directory to sources
      ln -s $srcdir dune

      # sanity check
      AS_IF([! test -r "dune/$ac_unique_file"],[
        AC_MSG_ERROR([Sanity check for symlink failed! Please send a bugreport to dune@dune-project.org])
      ])
    ])
  ],[
    # no symlinks possible... check name of directory
    AC_MSG_ERROR([No symlinks supported! You have to install dune. No inplace usage possible!])
  ])
])

AC_DEFUN([DUNE_WEB],
[
  # special variable to include the documentation into the website
  AC_ARG_WITH(duneweb,
    AS_HELP_STRING([--with-duneweb=PATH],[Only needed for website-generation, path to checked out version of dune-web]))

  AS_IF([test -n "$with_duneweb"],[
    AS_IF([test "x$with_duneweb" != "xno"],[
      # parameter is set. Check it
      AC_MSG_CHECKING([whether passed Dune-Web ($with_duneweb) directory appears correct])
      WEBTESTFILE="$with_duneweb/layout/default.wml"
      AS_IF([test -d "$with_duneweb" -a -e "$WEBTESTFILE"],[
         AC_MSG_RESULT([ok])
         # normalize path
         with_duneweb=`(cd $with_duneweb && pwd)` ;
      ],[
         AS_IF([test -d "$with_duneweb"],[
           AC_MSG_ERROR([$WEBTESTFILE not found in Dune-web dir $with_duneweb!])
         ],[
           AC_MSG_ERROR([Dune-Web directory $with_duneweb not found!])
         ])
         with_duneweb=no
      ])
    ])
     DUNE_ADD_SUMMARY_ENTRY([dune web],[$with_duneweb])
  ],[
    with_duneweb=no
  ])
  AC_SUBST(DUNEWEBDIR, $with_duneweb)
  AM_CONDITIONAL(DUNEWEB, [test "x$with_duneweb" != "xno"])
])
