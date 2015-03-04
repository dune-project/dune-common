# Determine flags necessary to compile multithreaded programs
#
# We simply defer to ACX_PTHREAD

AC_DEFUN([DUNE_STDTHREAD],[
  AC_REQUIRE([ACX_PTHREAD])
  AC_CACHE_CHECK([libraries needed for std::thread],
    [dune_cv_stdthread_libs],
    [dune_cv_stdthread_libs=$PTHREAD_LIBS])
  AC_CACHE_CHECK([linker flags needed for std::thread],
    [dune_cv_stdthread_ldflags],
    [dune_cv_stdthread_ldflags=$PTHREAD_CFLAGS])
  AC_CACHE_CHECK([compiler flags needed for std::thread],
    [dune_cv_stdthread_cppflags],
    [dune_cv_stdthread_cppflags=$PTHREAD_CFLAGS])

  AC_CACHE_CHECK([whether std::thread works],
    [dune_cv_stdthread_works],
    [
      AC_LANG_PUSH([C++])
      dune_save_CPPFLAGS=$CPPFLAGS
      dune_save_LDFLAGS=$LDFLAGS
      dune_save_LIBS=$LIBS
      CPPFLAGS="$CPPFLAGS $dune_cv_stdthread_cppflags"
      LDFLAGS="$LDFLAGS $dune_cv_stdthread_ldflags"
      LIBS="$dune_cv_stdthread_libs $LIBS"
      AC_RUN_IFELSE([DUNE_STDTHREAD_TESTPROG],
        [dune_cv_stdthread_works=yes],
        [dune_cv_stdthread_works=no],
        [dune_cv_stdthread_works=unknown])
      LIBS=$dune_save_LIBS
      LDFLAGS=$dune_save_LDFLAGS
      CPPFLAGS=$dune_save_CPPFLAGS
      AC_LANG_POP([C++])
    ])
  AS_CASE([$dune_cv_stdthread_works],
    [unknown], [AC_MSG_WARN([Cross compiling; cannot check whether std::thread works.  I am going to assume that the flags guessed above do work; if not, please adjust the cache variables dune_cv_stdthread_*])
                DUNE_STDTHREAD_WORKS=yes],
               [DUNE_STDTHREAD_WORKS=$dune_cv_stdthread_works])

  AC_SUBST([STDTHREAD_LIBS], ["${dune_cv_stdthread_libs}"])
  AC_SUBST([STDTHREAD_LDFLAGS], ["${dune_cv_stdthread_ldflags}"])
  AC_SUBST([STDTHREAD_CPPFLAGS], ["${dune_cv_stdthread_cppflags}"])
  AM_CONDITIONAL([STDTHREAD], [test "x$DUNE_STDTHREAD_WORKS" = xyes])
])

AC_DEFUN([DUNE_STDTHREAD_TESTPROG], [dnl
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
