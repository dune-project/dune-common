# tests for C++11 class std::thread
# the associated macro is called HAVE_STD_THREAD
# the automake conditional is STD_THREAD
# the linker flags are substituted into STD_THREAD_LDFLAGS
# the other substitution names STD_THREAD_* are reserved and currently empty
#
# Note: when using threads in a program, the linker flags need to be
# specified.  Otherwise, libstdc++ provides stubs for it's threading
# virtualization layer that result in confusing bugs.

AC_DEFUN([DUNE_STD_THREAD],[
  AC_REQUIRE([CXX11])
  AC_CACHE_CHECK([whether std::thread is declared], [dune_cv_std_thread_declared], [
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE(
      [DUNE_STD_THREAD_TESTPROG],
      [dune_cv_std_thread_declared=yes],
      [dune_cv_std_thread_declared=no])
    AC_LANG_POP([C++])
  ])
  if test "x$dune_cv_std_thread_declared" = xyes; then
    dune_std_thread_works=no
    DUNE_STD_THREAD_FLAGS
    DUNE_STD_THREAD_FLAGS([LDFLAGS=-pthread])
  fi
  AC_MSG_CHECKING([whether std::thread is supported])
  AC_MSG_RESULT([$dune_std_thread_works])
  if test "x$dune_std_thread_works" = xyes; then
    AC_DEFINE([HAVE_STD_THREAD], [1], [Define to 1 if std::thread is supported])
  fi
  AM_CONDITIONAL([STD_THREAD], [test "x$dune_std_thread_works" = xyes])
  AC_SUBST([STD_THREAD_LDFLAGS])
  DUNE_ADD_SUMMARY_ENTRY([std::thread],[$dune_std_thread_works])
])

#DUNE_STD_THREAD_FLAGS([settings])
AC_DEFUN([DUNE_STD_THREAD_FLAGS],[
  if ! test "x$dune_cv_std_thread_works" = xyes
  then
    AC_MSG_CHECKING([whether std::thread works m4_ifblank([$1], [out of the box], [with $1])])
    AC_LANG_PUSH([C++])
    dune_save_LDFLAGS=$LDFLAGS
    LDFLAGS=
    $1
    STD_THREAD_LDFLAGS=$LDFLAGS
    LDFLAGS="$dune_save_LDFLAGS $LDFLAGS"
    AC_RUN_IFELSE(
      [DUNE_STD_THREAD_TESTPROG],
      [dune_std_thread_works=yes],
      [
        dune_std_thread_works=no
        STD_THREAD_LDFLAGS=
      ])
    AC_LANG_POP([C++])
    LDFLAGS=$dune_save_LDFLAGS
    AC_MSG_RESULT([$dune_std_thread_works])
  fi
])

AC_DEFUN([DUNE_STD_THREAD_TESTPROG], [dnl
  AC_LANG_PROGRAM([[
    #include <thread>

    void f()
    {
      // do nothing
    }
    ]],
    [[
      std::thread t(f);
      t.join();
    ]])dnl
])
