dnl -*- autoconf -*-
# $Id$
# checks for dune-headers and everything they need

# TODO
#
# - use pkg-config if --enable-localdune is not provided

#   #export PKG_CONFIG_LIBDIR=$with_dune/dune
#  #PKG_CHECK_MODULES(DUNE, dune)  

# DUNE_VAR_IF (VAR, [TEXT], [IF-EQUAL], [IF-NOT-EQUAL])
#
# Compatibility wrapper around AS_VAR_IF.  If AS_VAR_IF is available, use it,
# otherwise emulate it.
AC_DEFUN([DUNE_VAR_IF],
[m4_ifdef([AS_VAR_IF],
  [AS_VAR_IF([$1], [$2], [$3], [$4])],
  [AS_IF([{ _dune_var_if_tmp="$2"
            eval "test x\"\$AS_ESCAPE([$1])\" = x\"\$_dune_var_if_tmp\""
          }], [$3], [$4])])
])

# DUNE_VAR_COPY (DEST, SRC)
#
# Compatibility wrapper around AS_VAR_COPY.  If AS_VAR_COPY is available, use
# it, otherwise emulate it.
AC_DEFUN([DUNE_VAR_COPY],
[m4_ifdef([AS_VAR_COPY],
  [AS_VAR_COPY([$1], [$2])],
  [eval "$1=\$$2"])
])

# DUNE_VAR_APPEND (VAR, TEXT)
#
# Compatibility wrapper around AS_VAR_APPEND.  If AS_VAR_APPEND is available,
# use it, otherwise emulate it.
AC_DEFUN([DUNE_VAR_APPEND],
[m4_ifdef([AS_VAR_APPEND],
  [AS_VAR_APPEND([$1], [$2])],
  [DUNE_VAR_COPY([_dune_va_tmp], [$1])
AS_VAR_SET([$1], ["$_dune_va_tmp"$2])])
])

# DUNE_ADD_ALL_PKG (TAG, [CPPFLAGS], [LDFLAGS], [LIBS])
#
# Add the flags and libs of an external library to the ALL_PKG_* variables,
# avoiding duplicates.
#
# TAG Tag used to avoid duplicate entries in the global variables.  This
#     should be the same in all invocations of this macro for the same
#     library, even for different modules.
#
# CPPFLAGS The preprocessor flags to add.
#
# LDFLAGS The linker flags to add.
#
# LIBS The libraries to add.
#
# All arguments allow shell substitutions.  Each argument should be suitable
# for use inside shell double quotes.  The flags given as argument here should
# not include the flags of dependend libraries that already call
# DUNE_ADD_ALL_PKG themselves; e.g. HDF5 should not add the DUNEMPI* flags.
#
# configure/shell variables:
#   ALL_PKG_CPPFLAGS
#   ALL_PKG_LDFLAGS
#   ALL_PKG_LIBS
AC_DEFUN([DUNE_ADD_ALL_PKG],
[AS_VAR_PUSHDEF([_dune_aap_TAG], [_dune_aap_tag_$1])dnl
AS_VAR_SET_IF([_dune_aap_TAG], [],
[DUNE_VAR_APPEND([ALL_PKG_CPPFLAGS], [" $2"])
DUNE_VAR_APPEND([ALL_PKG_LDFLAGS], [" $3"])
ALL_PKG_LIBS="$4 $ALL_PKG_LIBS"
AS_VAR_SET([_dune_aap_TAG], [1])
])
AS_VAR_POPDEF([_dune_aap_TAG])dnl
])

# DUNE_ADD_MODULE_DEPS (MODULE, TAG, [CPPFLAGS], [LDFLAGS], [LIBS])
#
# Add the flags and libs of an external library to the modules flags
# and libs and to the the DUNE_* and ALL_PKG_* families of global flags and
# libs.
#
# MODULE The name to the DUNE module to add the flags for.  It should be
#     lowercase with components seperated by dashes (like "dune-common").
#
# TAG Tag used to avoid duplicate entries in the global variables.  This
#     should be the same in all invocations of this macro for the same
#     library, even for different modules.
#
# CPPFLAGS The preprocessor flags to add.
#
# LDFLAGS The linker flags to add.
#
# LIBS The libraries to add.
#
# All arguments allow shell substitutions.  Each argument should be suitable
# for use inside shell double quotes.
#
# configure/shell variables (MODULE_upper denotes an uppercase version of
# MODULE as obtained by AS_TR_CPP):
#   {MODULE_upper}_DEPS_CPPFLAGS
#   {MODULE_upper}_DEPS_LDFLAGS
#   {MODULE_upper}_DEPS_LIBS
#   ALL_PKG_CPPFLAGS
#   ALL_PKG_LDFLAGS
#   ALL_PKG_LIBS
#   DUNE_CPPFLAGS
#   DUNE_LDFLAGS
#   DUNE_LIBS
AC_DEFUN([DUNE_ADD_MODULE_DEPS],
[# Add module specific flags and libs
AS_VAR_PUSHDEF([_dune_amd_CPPFLAGS], [AS_TR_CPP([$1])_DEPS_CPPFLAGS])dnl
DUNE_VAR_COPY([_dune_amd_tmp], [_dune_amd_CPPFLAGS])
AS_VAR_SET([_dune_amd_CPPFLAGS], ["$3 "$_dune_amd_tmp])
AS_VAR_POPDEF([_dune_amd_CPPFLAGS])dnl

AS_VAR_PUSHDEF([_dune_amd_LDFLAGS], [AS_TR_CPP([$1])_DEPS_LDFLAGS])dnl
DUNE_VAR_COPY([_dune_amd_tmp], [_dune_amd_LDFLAGS])
AS_VAR_SET([_dune_amd_LDFLAGS], ["$4 "$_dune_amd_tmp])
AS_VAR_POPDEF([_dune_amd_LDFLAGS])dnl

AS_VAR_PUSHDEF([_dune_amd_LIBS], [AS_TR_CPP([$1])_DEPS_LIBS])dnl
DUNE_VAR_COPY([_dune_amd_tmp], [_dune_amd_LIBS])
AS_VAR_SET([_dune_amd_LIBS], ["$5 "$_dune_amd_tmp])
AS_VAR_POPDEF([_dune_amd_LIBS])dnl

# add flags and libs to the ALL_PKG_* family
DUNE_ADD_ALL_PKG([$2], [\${_DUNE_MODULE[]_CPPFLAGS}], [\${_DUNE_MODULE[]_LDFLAGS}], [\${_DUNE_MODULE[]_LIBS}])

# add flags and libs to the DUNE_* family
AS_VAR_PUSHDEF([_dune_amd_TAG], [_dune_amd_tag_$2])dnl
AS_VAR_SET_IF([_dune_amd_TAG], ,
[DUNE_VAR_APPEND([DUNE_CPPFLAGS], [" $3"])
DUNE_VAR_APPEND([DUNE_LDFLAGS], [" $4"])

# add flags to the deprecated DUNE_PKG_* family as well
DUNE_VAR_APPEND([DUNE_PKG_CPPFLAGS], [" $3"])
DUNE_VAR_APPEND([DUNE_PKG_LDFLAGS], [" $4"])
DUNE_PKG_LIBS="$5 $DUNE_PKG_LIBS"
AS_VAR_SET([_dune_amd_TAG], [1])
])
AS_VAR_POPDEF([_dune_amd_TAG], [_dune_amd_tag_$2])dnl
])

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
  AS_IF([test -n "$_DUNE_MODULE[]_VERSION" -a "x$with_[]_dune_module" != "xno"],[
    result="version $_DUNE_MODULE[]_VERSION"
  ],[
    result="$with_[]_dune_module"
  ])
  DUNE_ADD_SUMMARY_MOD_ENTRY(_dune_name,[$result],[$_DUNE_MODULE[]_ROOT])
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

  AC_SUBST(_DUNE_MODULE[]_VERSION,_dune_version)
  AC_SUBST(_DUNE_MODULE[]_VERSION_MAJOR,_dune_sub_version(_dune_version,1))
  AC_SUBST(_DUNE_MODULE[]_VERSION_MINOR,_dune_sub_version(_dune_version,2))
  AC_SUBST(_DUNE_MODULE[]_VERSION_REVISION,_dune_sub_version(_dune_version,3))

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
#        handed to AC_LINK_IFELSE/AC_LANG_PROGRAM unchanged as the
#        FUNCTION-BODY argument, so it may contain more complex stuff than a
#        simple symbol.
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
#   {MODULE}_ROOT, {MODULE}_LIBDIR
#   HAVE_{MODULE} (1 or 0)
#   with_{module} ("yes" or "no")
#   DUNE_CPPFLAGS, DUNE_LDFLAGS, DUNE_LIBS (adds the modules values here,
#         substitution done by DUNE_CHECK_ALL)
#   ALL_PKG_CPPFLAGS, ALL_PKG_LDFLAGS, ALL_PKG_LIBS (adds the modules values
#         here, substitution done by DUNE_CHECK_ALL)
#   DUNE_PKG_CPPFLAGS, DUNE_PKG_LDFLAGS, DUNE_PKG_LIBS (deprecated, adds the
#         modules values here)
#   {MODULE}_VERSION
#   {MODULE}_VERSION_MAJOR
#   {MODULE}_VERSION_MINOR
#   {MODULE}_VERSION_REVISION
#
# configure substitutions/makefile variables:
#   {MODULE}_CPPFLAGS, {MODULE}_LDFLAGS, {MODULE}_LIBS
#   {MODULE}_ROOT
#   {MODULE}_LIBDIR (only if modules provides a library)
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

  # if DUNE_LDFLAGS have a pre-assigned value (e.g. -all-static), 
  # also store it in DUNE_LDFLAGS_TMP to be consistent with DUNE_LDFLAGS
  # the same holds for the other TMP variables, this is only to be done 
  # if these TMP variables are empty to avoid double apprearance  
  if test "x$DUNE_LDFLAGS_TMP" = "x"; then
    DUNE_LDFLAGS_TMP="$DUNE_LDFLAGS"
  fi
  if test "x$DUNE_CPPFLAGS_TMP" = "x"; then
    DUNE_CPPFLAGS_TMP="$DUNE_CPPFLAGS"
  fi
  if test "x$DUNE_LIBS_TMP" = "x"; then
    DUNE_LIBS_TMP="$DUNE_LIBS"
  fi

  ##
  ## Where is the module $1?
  ##

  AC_MSG_CHECKING([for $1 installation or source tree])

  # is a directory set?
  AS_IF([test -z "$with_[]_dune_module"],[
    #
    # initialize variables for lib
    #
    _DUNE_MODULE[]_LIBDIR=""
    _dune_cm_LDFLAGS=""
    _dune_cm_LIBS=""
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
      _dune_cm_CPPFLAGS="`$PKG_CONFIG --cflags _dune_name` -I./" 2>/dev/null
      _DUNE_MODULE[]_ROOT="`$PKG_CONFIG --variable=prefix _dune_name`" 2>/dev/null 
      _DUNE_MODULE[]_VERSION="`$PKG_CONFIG --modversion _dune_name`" 2>/dev/null
      _DUNE_MODULE[]_LIBDIR=`$PKG_CONFIG --variable=libdir _dune_name 2>/dev/null`
      _dune_cm_LIBS=`$PKG_CONFIG --libs _dune_name 2>/dev/null`
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
      _DUNE_MODULE[]_LIBDIR="$_DUNE_MODULE[]_ROOT/lib"

      # expand search path (otherwise empty CPPFLAGS)
      AS_IF([test -d "$_DUNE_MODULE[]_ROOT/include/dune"],[
        # Dune was installed into directory given by with-dunecommon
        _dune_cm_CPPFLAGS="-I$_DUNE_MODULE[]_ROOT/include"
        _DUNE_MODULE[]_BUILDDIR=_DUNE_MODULE[]_ROOT
        _DUNE_MODULE[]_VERSION="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$_DUNE_MODULE[]_ROOT/lib/pkgconfig $PKG_CONFIG --modversion _dune_name`" 2>/dev/null
		_dune_cm_LIBS="-L$_DUNE_MODULE[]_LIBDIR -l[]_dune_lib"
      ],[
        _DUNE_MODULE[]_SRCDIR=$_DUNE_MODULE[]_ROOT
        echo "testing $_DUNE_MODULE[]_ROOT/CMakeCache.txt"
        # extract src and build path from Makefile or CMakeCache.txt, if found
        AS_IF([test -f $_DUNE_MODULE[]_ROOT/CMakeCache.txt],[
          _DUNE_MODULE[]_SRCDIR="`sed -ne '/^[]_dune_name[]_SOURCE_DIR:STATIC=/{s/^[]_dune_name[]_SOURCE_DIR:STATIC=//; p;}' $_DUNE_MODULE[]_ROOT/CMakeCache.txt`"
          echo srcdir=$_DUNE_MODULE[]_SRCDIR
                ],[test -f $_DUNE_MODULE[]_ROOT/Makefile],[
          _DUNE_MODULE[]_SRCDIR="`sed -ne '/^abs_top_srcdir = /{s/^abs_top_srcdir = //; p;}' $_DUNE_MODULE[]_ROOT/Makefile`"
		])
        _dune_cm_CPPFLAGS="-I$_DUNE_MODULE[]_SRCDIR"
        _DUNE_MODULE[]_VERSION="`grep Version $_DUNE_MODULE[]_SRCDIR/dune.module | sed -e 's/^Version: *//'`" 2>/dev/null
        AS_IF([test -f "$_DUNE_MODULE[]_LIBDIR[]/lib[]_dune_lib[].la"], [
          # local modules is linked directly via the .la file
          _dune_cm_LIBS="$_DUNE_MODULE[]_LIBDIR[]/lib[]_dune_lib[].la"],[
          # No libtool file. This indicates a module build with CMake. Fall back to direct linking
          _dune_cm_LIBS="-L$_DUNE_MODULE[]_LIBDIR[]/ -l[]_dune_lib[]"
        ])
      ])
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

  CPPFLAGS="$ac_save_CPPFLAGS $DUNE_CPPFLAGS_TMP $DUNE_PKG_CPPFLAGS $_dune_cm_CPPFLAGS"
  ##  
  ## check for an arbitrary header
  ##
  AC_CHECK_HEADER([dune/[]_dune_header],
    [HAVE_[]_DUNE_MODULE=1],
    [HAVE_[]_DUNE_MODULE=0
     AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
       AC_MSG_WARN([$_DUNE_MODULE[]_ROOT does not seem to contain a valid _dune_name (dune/[]_dune_header not found)])
     ])
    ]
  )

  ##
  ## check for lib (if lib name was provided)
  ##
  ifelse(_dune_symbol,,
    # clear libs flags and inform the user
    _DUNE_MODULE[]_LIBDIR=""
    _dune_cm_LIBS=""
    AC_MSG_NOTICE([_dune_name does not provide libs]),

    AS_IF([test "x$enable_dunelibcheck" = "xno"],[
      AC_MSG_WARN([library check for _dune_name is disabled. DANGEROUS!])
    ],[
      AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[
        # save current LDFLAGS
        ac_save_CXX="$CXX"
        AC_CACHE_CHECK([for lib[]_dune_lib], dune_cv_lib[]_dune_lib, [
          # Use $CXX $DUNE_LDFLAGS_TMP as link command, as the latter might 
          # contain the -static option to force static linkage
          ac_cxx_ld=`echo $ac_save_CXX | sed -e "s@$CXX@$CXX $DUNE_LDFLAGS_TMP@"`

          # define LTCXXLINK like it will be defined in the Makefile
          CXX="./libtool --tag=CXX --mode=link $ac_cxx_ld "
          
          # use module LDFLAGS
          LDFLAGS="$ac_save_LDFLAGS $DUNE_PKG_LDFLAGS $_dune_cm_LDFLAGS"
          LIBS="$_dune_cm_LIBS $DUNE_LIBS_TMP $LIBS"

          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
              [#]include<dune/[]_dune_header>,
              _dune_symbol)],
            [dune_cv_lib[]_dune_lib=yes],
            [dune_cv_lib[]_dune_lib=no])
        ])

        AS_IF([test "x$dune_cv_lib[]_dune_lib" = "xyes"], [
          HAVE_[]_DUNE_MODULE=1
          ],[
          HAVE_[]_DUNE_MODULE=0
          AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
            AC_MSG_WARN([$with_[]_dune_module does not seem to contain a valid _dune_name (failed to link with lib[]_dune_lib[].la)])
          ])
        ])
      ])

      # reset variables
      CXX="$ac_save_CXX"
    ])
  )

  # did we succeed?
  AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[
    # add the module's own flags and libs to the modules and the global
    # variables
    DUNE_ADD_MODULE_DEPS(m4_defn([_dune_name]), m4_defn([_dune_name]),
        [$_dune_cm_CPPFLAGS], [$_dune_cm_LDFLAGS], [$_dune_cm_LIBS])

    # set variables for our modules
    AC_SUBST(_DUNE_MODULE[]_CPPFLAGS, "$_dune_cm_CPPFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LDFLAGS, "$_dune_cm_LDFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LIBS, "$_dune_cm_LIBS")
    AC_SUBST(_DUNE_MODULE[]_ROOT, "$_DUNE_MODULE[]_ROOT")
    ifelse(m4_defn([_dune_symbol]),,
      [],
      [AC_SUBST(_DUNE_MODULE[]_LIBDIR)
    ])
    AC_DEFINE(HAVE_[]_DUNE_MODULE, 1, [Define to 1 if] _dune_name [was found])

    DUNE_PARSE_MODULE_VERSION(_dune_name, $_DUNE_MODULE[]_VERSION)

    # set DUNE_* variables
    # This should actually be unneccesary, but I'm keeping it in here for now
    # for backward compatibility
    DUNE_CPPFLAGS="$DUNE_CPPFLAGS \${_DUNE_MODULE[]_CPPFLAGS}"
    DUNE_LDFLAGS="$DUNE_LDFLAGS \${_DUNE_MODULE[]_LDFLAGS}"
    DUNE_LIBS="\${_DUNE_MODULE[]_LIBS} $DUNE_LIBS"

    # the TMP version is used during the checks 
    DUNE_CPPFLAGS_TMP="$DUNE_CPPFLAGS_TMP $_DUNE_MODULE[]_CPPFLAGS"
    DUNE_LDFLAGS_TMP="$DUNE_LDFLAGS_TMP $_DUNE_MODULE[]_LDFLAGS"
    DUNE_LIBS_TMP="$_DUNE_MODULE[]_LIBS $DUNE_LIBS_TMP"
    
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
  LDFLAGS="$ac_save_LDFLAGS"
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
