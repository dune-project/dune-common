# Check if __attribute__((unused)) is supported

AC_DEFUN([DUNE_CHECKUNUSED],[
    AC_CACHE_CHECK([for __attribute__((unused))], dune_cv_attribute_unused, [
        AC_LANG_PUSH([C++])
        AC_TRY_COMPILE([#define UNUSED __attribute__((unused))
                        void f(int a UNUSED, int UNUSED)
                        {
                          int UNUSED b;
                        }],
                       [],
                        dune_cv_attribute_unused="yes",
                        dune_cv_attribute_unused="no")
        AC_LANG_POP([C++])
    ])

    AS_IF([test "x$dune_cv_attribute_unused" = "xyes"],
      [AC_DEFINE_UNQUOTED(HAS_ATTRIBUTE_UNUSED,
                          1,
                          [does the compiler support __attribute__((unused))?])],)

    AH_BOTTOM([#include <dune/common/unused.hh>])
])
