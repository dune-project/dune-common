dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl We check two things: where is the memory include file, and in what
dnl namespace does shared_ptr reside.
dnl We include AC_COMPILE_IFELSE for all the combinations we've seen in the
dnl wild:
dnl 
dnl  GCC 4.3: namespace: std::  #include <memory>
dnl  GCC 4.2: namespace: tr1::  #include <tr1/memory>
dnl  GCC 4.2: namespace: boost::  #include <boost/shared_ptr.hpp>
dnl
dnl We define one of HAVE_HAVE_TR1_SHARED_PTR or HAVE_BOOST_SHARED_PTR
dnl depending on location, SHARED_PTR_HEADER to be the header with enclosing
dnl brackety braces in which shared_ptr is defined and SHARED_PTR_NAMESPACE to 
dnl be the namespace in
dnl which shared_ptr is defined.
dnl 

AC_DEFUN([SHARED_PTR],[
dnl  AC_REQUIRE([PANDORA_CHECK_CXX_STANDARD])
  AC_REQUIRE([DUNE_TR1_HEADERS])
  AC_LANG_PUSH(C++)
  AS_IF([test "x$enable_tr1_headers" != "xno"],
    [AC_CHECK_HEADERS([memory tr1/memory])])
  AC_CHECK_HEADERS([boost/shared_ptr.hpp])
  AC_CACHE_CHECK([the location of shared_ptr header file],
    [ac_cv_shared_ptr_h],[
      for namespace in std tr1 std::tr1 boost
      do
        AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM([[
#include <string>

using $namespace::shared_ptr;
using namespace std;
            ]],[[
shared_ptr<string> test_ptr(new string("test string"));
            ]])],
            [
              ac_cv_shared_ptr_namespace="${namespace}"
              ac_cv_shared_ptr_header=missing
              break
            ],[
	      ac_cv_shared_ptr_namespace=missing
              ac_cv_shared_ptr_header=missing
            ])
        for header in memory tr1/memory boost/shared_ptr.hpp; do
          AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[
# include <$header>
#include <string>

using $namespace::shared_ptr;
using namespace std;
              ]],[[
shared_ptr<string> test_ptr(new string("test string"));
              ]])],
              [
                ac_cv_shared_ptr_namespace="${namespace}"
                ac_cv_shared_ptr_header="<${header}>"
                break
              ],[
                ac_cv_shared_ptr_namespace=missing
                ac_cv_shared_ptr_header=missing
              ])
         done
         if test "$ac_cv_shared_ptr_namespace" != "missing"; then break; fi
       done
  ])
  AS_IF([ test "x$ac_cv_shared_ptr_namespace" = xmissing ],
    [], [
      SHARED_PTR_NAMESPACE=${ac_cv_shared_ptr_namespace}
      AC_DEFINE_UNQUOTED([SHARED_PTR_NAMESPACE],
                         ${ac_cv_shared_ptr_namespace},
                         [The namespace in which SHARED_PTR can be found])
    ]
  )
  AS_IF([ test "x$ac_cv_shared_ptr_header" = xmissing ],
    [], [
      SHARED_PTR_HEADER=${ac_cv_shared_ptr_header}
      AC_DEFINE_UNQUOTED([SHARED_PTR_HEADER],
                         ${ac_cv_shared_ptr_header},
                         [The header in which SHARED_PTR can be found])
    ]
  )
  AC_LANG_POP()
])
