AC_DEFUN([BOOST_MAKE_SHARED_HPP],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_LANG_PUSH([C++])
     AC_CHECK_HEADER([boost/make_shared.hpp],
                     [AC_DEFINE([HAVE_BOOST_MAKE_SHARED_HPP], [1],
                        [Define to 1 if you have <boost/make_shared.hpp>.])])
  AC_LANG_POP
])
