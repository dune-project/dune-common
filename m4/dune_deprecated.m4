# $Id$

# Check for the right way to create the deprecation warning

AC_DEFUN([DUNE_CHECKDEPRECATED],[
	AC_MSG_CHECKING([for __attribute__((deprecated))])
        AC_LANG_PUSH([C++])
        AC_TRY_COMPILE([#define DEP __attribute__((deprecated))
                    class bar { bar() DEP; };
                    class peng { } DEP;
                    template <class T>
                    class t_bar { t_bar() DEP; };
                    template <class T>
                    class t_peng { t_peng() {}; } DEP;
                    void foo() DEP;
                    void foo() {};],[],
                                  [HAS_ATTRIBUTE_DEPRECATED="yes"
                    AC_MSG_RESULT(yes)],
                                  [HAS_ATTRIBUTE_DEPRECATED="no"
                    AC_MSG_RESULT(no)])

        AC_LANG_POP([C++])

	AC_MSG_CHECKING([for __attribute__((deprecated("message")))])
        AC_LANG_PUSH([C++])
        AC_TRY_COMPILE([#define DEP __attribute__((deprecated("fireworks!")))
                    class bar { bar() DEP; };
                    class peng { } DEP;
                    template <class T>
                    class t_bar { t_bar() DEP; };
                    template <class T>
                    class t_peng { t_peng() {}; } DEP;
                    void foo() DEP;
                    void foo() {};],[],
                                  [HAS_ATTRIBUTE_DEPRECATED_MSG="yes"
                     AC_MSG_RESULT(yes)],
                                  [HAS_ATTRIBUTE_DEPRECATED_MSG="no"
                     AC_MSG_RESULT(no)])
         AC_LANG_POP([C++])
 
    if test "$HAS_ATTRIBUTE_DEPRECATED" = "yes"; then
        AC_DEFINE_UNQUOTED(HAS_ATTRIBUTE_DEPRECATED, 1,
                          [does the compiler support __attribute__((deprecated))?])
    fi

    if test "$HAS_ATTRIBUTE_DEPRECATED_MSG" = "yes"; then
        AC_DEFINE_UNQUOTED(HAS_ATTRIBUTE_DEPRECATED_MSG, 1,
                          [does the compiler support __attribute__((deprecated("text"))?])
    fi

    AH_BOTTOM([#include <dune/common/deprecated.hh>])
])
