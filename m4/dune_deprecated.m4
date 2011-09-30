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
                                  [DUNE_DEPRECATED="__attribute__((deprecated))"
                                   DUNE_DEPRECATED_MSG="__attribute__((deprecated(msg) ))"
                     AC_MSG_RESULT(yes)],
                                  [DUNE_DEPRECATED=""
                                   DUNE_DEPRECATED_MSG=""
                     AC_MSG_RESULT(no)])

         AC_LANG_POP([C++])
 
    AC_DEFINE_UNQUOTED(DUNE_DEPRECATED, $DUNE_DEPRECATED,
                      [how to create a deprecation warning])
    AC_DEFINE_UNQUOTED(DUNE_DEPRECATED_MSG(msg), $DUNE_DEPRECATED_MSG,
                      [how to create a deprecation warning with an additional message])
])
