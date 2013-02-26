# Check if the compiler supports the abi::__cxa_demangle function required to
# make the type names returned by typeid() human-readable
AC_DEFUN([DUNE_CHECK_CXA_DEMANGLE],[
    AC_CACHE_CHECK([for abi::__cxa_demangle], dune_cv_cxa_demangle, [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <cxxabi.h>
            ]],
            [[
    int foobar = 0;
    const char *foo = typeid(foobar).name();
    int status;
    char *demangled = abi::__cxa_demangle( foo, 0, 0, &status );
            ]]
          )],
          dune_cv_cxa_demangle="yes",
          dune_cv_cxa_demangle="no")
        AC_LANG_POP([C++])
    ])

    AS_IF([test "x$dune_cv_cxa_demangle" = "xyes"],
      [AC_DEFINE_UNQUOTED(HAVE_CXA_DEMANGLE,
                          1,
                          [does the compiler support abi::__cxa_demangle])])
])
