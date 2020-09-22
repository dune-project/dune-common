include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature is_indexable")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++14")
  check_cxx_source_compiles("
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
    struct _is_indexable<T,I,typename std::enable_if<(sizeof(declval<T>()[declval<I>()]) > 0),int>::type>
      : public std::true_type
    {};

  }

  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public detail::_is_indexable<T,I>
  {};

  struct foo_type {};

  int main()
  {
    double x;
    std::array<double,4> y;
    double z[5];
    foo_type f;

    static_assert(not is_indexable<decltype(x)>::value,\"scalar type\");
    static_assert(is_indexable<decltype(y)>::value,\"indexable class\");
    static_assert(is_indexable<decltype(z)>::value,\"array\");
    static_assert(not is_indexable<decltype(f)>::value,\"not indexable class\");
    static_assert(not is_indexable<decltype(y),foo_type>::value,\"custom index type\");

    return 0;
  }
  "  DUNE_HAVE_CXX_IS_INDEXABLE_SUPPORT
  )
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_IS_INDEXABLE_SUPPORT)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_IS_INDEXABLE_SUPPORT)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_14)
    message(STATUS "Looking for optional compiler feature is_indexable - found")
  else ()
    message(STATUS "Looking for optional compiler feature is_indexable - not found")
  endif ()
endfunction (add_feature)
