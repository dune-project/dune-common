# $Id$
# searches for UG headers and libs

AC_DEFUN(AM_PATH_UG,
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])

  AC_ARG_WITH(ug,
    AC_HELP_STRING([--with-ug=PATH],[directory with UG inside]),
dnl     expand tilde / other stuff
    eval with_ug=$with_ug
dnl     extract absolute path
dnl    eval with_ug=`cd $with_ug ; pwd`
 
if test x$with_ug != x ; then 
 # store values
 ac_save_CFLAGS="$CFLAGS"
 ac_save_LDFLAGS="$LDFLAGS"
 ac_save_LIBS="$LIBS"
 LIBS=""

 # start building variables

 # use special UG-lib-path if it's set
 if test x$with_ug != x ; then
   LDFLAGS="$LDFLAGS -L$with_ug/lib"
   UG_LDFLAGS="$LDFLAGS"
 fi

 # read ug.conf
 # IF = X|S
 # DOM_MODULE -> -D...
 # GRAPE
 # DIM? 

 # nach den UG-Unterlibs suchen

 AC_CHECK_LIB([devS], [UserWrite], UG_DEVLIB="-ldevS")

 # devX oder devS
 AC_CHECK_LIB([Xaw], [XawInitializeWidgetSet],,,[$X_LIBS])  
 AC_CHECK_LIB([devX], [UserWrite], UG_DEVLIB="-ldevX $X_PRE_LIBS $X_LIBS -lX11 -lXaw $X_EXTRA_LIBS",,[$X_PRE_LIBS $X_LIBS -lX11 -lXaw $X_EXTRA_LIBS])
 
 if test x$UG_DEVLIB != x ; then

 fi 
 
 # wurde irgendeine dev-lib gefunden?
 if test "x$UG_DEVLIB" = "x" ; then
   AC_MSG_WARN([No UG device-lib found!])
 fi

 AC_CHECK_LIB([domS$with_dim], [InitDom],,,[$X_LIBS])
 AC_CHECK_LIB([gg$with_dim], [InitGG],,,[$X_LIBS])

 
 # grapeOFF oder nicht
 # -> alles in eine lib-Zeile packen

 # jetzt ist alles beisammen
 AC_CHECK_LIB([ug$with_dim],[InitUg],,,[-lgrapeOFF$with_dim -ldevX -ldomS$with_dim -lgg$with_dim -lm ])

 UG_LIBS="$LIBS"
 AC_SUBST(UG_LIBS, $UG_LIBS)

 # reset values					    
 CFLAGS="$ac_save_CFLAGS"
 LIBS="$ac_save_LIBS"
 LDFLAGS="$ac_save_LDFLAGS"
fi

])