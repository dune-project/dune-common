# Check if the 'final' keyword is supported

AC_DEFUN([DUNE_CHECKFINAL],[
    AC_CACHE_CHECK([for C++11 keyword 'final'], dune_cv_keyword_final, [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
                        struct Foo
                        {
                          virtual void foo() final;
                        };
                          ]],
                          [])],
                        dune_cv_keyword_final="yes",
                        dune_cv_keyword_final="no")
        AC_LANG_POP([C++])
    ])

    AS_IF([test "x$dune_cv_keyword_final" = "xyes"],
      [AC_DEFINE_UNQUOTED(HAVE_KEYWORD_FINAL,
                          1,
                          [does the compiler support the keyword 'final'?])],)
])
