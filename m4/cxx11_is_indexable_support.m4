# tests whether the compiler properly supports testing for operator[]
# the associated macro is called HAVE_IS_INDEXABLE_SUPPORT

AC_DEFUN([DUNE_CXX11_IS_INDEXABLE_SUPPORT_CHECK],[
  AC_CACHE_CHECK([whether the compiler properly supports testing for operator[[]]],
    dune_cv_cxx11_is_indexable_support,
    [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CXX11])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(
        [
          #include <utility>
          #include <type_traits>
          #include <array>

          template <class T>
          typename std::add_rvalue_reference<T>::type declval();

          namespace detail {

            template<typename T, typename I, typename = int>
            struct _is_indexable
              : public std::false_type
            {};

            template<typename T, typename I>
            struct _is_indexable<T,I,typename std::enable_if<(sizeof(declval<T>()[[declval<I>()]]) > 0),int>::type>
              : public std::true_type
            {};

          }

          template<typename T, typename I = std::size_t>
          struct is_indexable
            : public detail::_is_indexable<T,I>
          {};

          struct foo_type {};

        ],[
          double x;
          std::array<double,4> y;
          double z[[5]];
          foo_type f;

          static_assert(not is_indexable<decltype(x)>::value,"scalar type");
          static_assert(is_indexable<decltype(y)>::value,"indexable class");
          static_assert(is_indexable<decltype(z)>::value,"array");
          static_assert(not is_indexable<decltype(f)>::value,"not indexable class");
          static_assert(not is_indexable<decltype(y),foo_type>::value,"custom index type");

          return 0;
        ])],
      dune_cv_cxx11_is_indexable_support=yes,
      dune_cv_cxx11_is_indexable_support=no)
    AC_LANG_POP
  ])
  if test "$dune_cv_cxx11_is_indexable_support" = yes; then
    AC_DEFINE(HAVE_IS_INDEXABLE_SUPPORT, 1,
      [Define to 1 if the compiler properly supports testing for operator[[]]])
  fi
])
