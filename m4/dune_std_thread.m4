# tests for C++11 class std::thread
# the associated macro is called HAVE_STD_THREAD
# the automake conditional is STD_THREAD

AC_DEFUN([DUNE_STD_THREAD],[
  AC_REQUIRE([GXX0X])
  AC_CACHE_CHECK([whether std::thread is supported], [dune_cv_std_thread_supported], [
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
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
        ]])],
      [dune_cv_std_thread_supported=yes],
      [dune_cv_std_thread_supported=no])
    AC_LANG_POP([C++])
  ])
  if test "x$dune_cv_std_thread_supported" = xyes; then
    AC_DEFINE([HAVE_STD_THREAD], [1], [Define to 1 if std::thread is supported])
  fi
  AM_CONDITIONAL([STD_THREAD], [test "x$dune_cv_std_thread_supported" = xyes])
])
