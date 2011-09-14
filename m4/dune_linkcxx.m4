# $Id$
# tries to link c++ objects

AC_DEFUN([DUNE_LINKCXX],[

AC_LANG_PUSH([C++])

AC_MSG_CHECKING([whether compiling C++ code works])
AC_COMPILE_IFELSE([
  AC_LANG_SOURCE(
    [ #include <iostream>
      int main (int argc, char** argv) { 
      std::cout << std::endl;
      }])],
  [ AC_MSG_RESULT([yes]);
    AC_MSG_CHECKING([whether linking C++ objects works]);
	# link from object file
	ac_old_link=$ac_link
	ac_link=`echo "$ac_old_link" | sed "s/conftest\.$ac_ext/conftest\.$ac_objext/"`
	# disable deletion of object file
	ac_old_objext=$ac_objext;
	ac_objext="";
    AC_LINK_IFELSE([],
      [ AC_MSG_RESULT([yes])],
      [ AC_MSG_RESULT([no])
        AC_MSG_ERROR([Could not link C++ code])])
	ac_objext=$ac_old_objext;
    ac_link=$ac_old_link;],
  [ AC_MSG_RESULT([no])
    AC_MSG_ERROR([Could not compile C++ code])]
)

AC_LANG_POP([C++])

])
