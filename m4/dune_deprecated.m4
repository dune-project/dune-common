# $Id$

# Check for the right way to create the deprecation warning

AC_DEFUN([DUNE_CHECKDEPRECATED],[
    AC_CACHE_CHECK([for __attribute__((deprecated))], dune_cv_attribute_deprecated, [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
                    #define DEP __attribute__((deprecated))
                    class bar { bar() DEP; };
                    class peng { } DEP;
                    template <class T>
                    class t_bar { t_bar() DEP; };
                    template <class T>
                    class t_peng { t_peng() {}; } DEP;
                    void foo() DEP;
                    void foo() {};
                  ]],
                  [])],
             dune_cv_attribute_deprecated="yes",
             dune_cv_attribute_deprecated="no")
        AC_LANG_POP([C++])
    ])

    AC_CACHE_CHECK([for __attribute__((deprecated("message")))], 
        dune_cv_attribute_deprecated_message, [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
                    #define DEP __attribute__((deprecated("fireworks!")))
                    class bar { bar() DEP; };
                    class peng { } DEP;
                    template <class T>
                    class t_bar { t_bar() DEP; };
                    template <class T>
                    class t_peng { t_peng() {}; } DEP;
                    void foo() DEP;
                    void foo() {};
                  ]],
                  [])],
            dune_cv_attribute_deprecated_message="yes",
            dune_cv_attribute_deprecated_message="no")
        AC_LANG_POP([C++])
    ])
 
    if test "$dune_cv_attribute_deprecated" = "yes"; then
        AC_DEFINE_UNQUOTED(HAS_ATTRIBUTE_DEPRECATED, 1,
                          [does the compiler support __attribute__((deprecated))?])
    fi

    if test "$dune_cv_attribute_deprecated_message" = "yes"; then
        AC_DEFINE_UNQUOTED(HAS_ATTRIBUTE_DEPRECATED_MSG, 1,
                          [does the compiler support __attribute__((deprecated("message"))?])
    fi

    AH_BOTTOM([#include <dune/common/deprecated.hh>])
])
