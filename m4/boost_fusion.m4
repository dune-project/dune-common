# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
AC_DEFUN([DUNE_BOOST_FUSION],
[
        AC_REQUIRE([AC_PROG_CC])
        CPPFLAGS_SAVED="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        export CPPFLAGS

        LDFLAGS_SAVED="$LDFLAGS"
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::FUSION library is available,
                               dune_cv_boost_fusion,
                               [AC_LANG_PUSH([C++])
                               AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[ @%:@include <boost/fusion/container.hpp>
                                  ]],
                                  [[
                                    boost::fusion::vector<int,char,double> v;
                                    return 0;
                                   ]]),
                               dune_boost_fusion=yes, dune_boost_fusion=no)
                               AC_LANG_POP([C++])])
         if test "x$dune_boost_filesystem" = "xyes"; then
               AC_DEFINE(HAVE_BOOST_FUSION,,[define if the Boost::Fusion headers are available])
         fi
         CPPFLAGS="$CPPFLAGS_SAVED"
         LDFLAGS="$LDFLAGS_SAVED"
])
