#ifndef DUNE_COMMON_STD_CONCEPTS_HH
#define DUNE_COMMON_STD_CONCEPTS_HH

#ifndef DUNE_HAVE_CXX_CONCEPTS
#error "<dune/common/std/concepts.hh> header shall not be included without language support for concepts (i.e. DUNE_HAVE_CXX_CONCEPTS)."
#endif

#if __has_include(<concepts>)
#include <concepts>

namespace Dune {
namespace Std {

  // There is no alias declaration for concepts, hence, we have to
  // actually write down concepts that are forwared to the <concepts> implementation
  #define DUNE_CONCEPT_ALIAS_1(name)        \
  template<class T>                         \
  concept name = std::name<T>;

  #define DUNE_CONCEPT_ALIAS_2(name)        \
  template<class T0, class T1>              \
  concept name = std::name<T0,T1>;

  #define DUNE_CONCEPT_ALIAS_1_VAR(name)    \
  template<class T, class... Args>          \
  concept name = std::name<T,Args...>;

  DUNE_CONCEPT_ALIAS_2(convertible_to)
  DUNE_CONCEPT_ALIAS_1(destructible)
  DUNE_CONCEPT_ALIAS_1_VAR(constructible_from)
  DUNE_CONCEPT_ALIAS_1(default_initializable)
  DUNE_CONCEPT_ALIAS_1(move_constructible)
  DUNE_CONCEPT_ALIAS_1(copy_constructible)

}
}

#else
namespace Dune {
namespace Std {

  // Fallback implementation for the concepts standard library
  // https://en.cppreference.com/w/cpp/header/concepts

  template <class From, class To>
  concept convertible_to =
    std::is_convertible_v<From, To> &&
    requires(std::add_rvalue_reference_t<From> (&f)()) {
      static_cast<To>(f());
    };

  template < class T >
  concept destructible = std::is_nothrow_destructible_v<T>;

  template < class T, class... Args >
  concept constructible_from =
    destructible<T> && std::is_constructible_v<T, Args...>;

  template<class T>
  concept default_initializable =
      constructible_from<T> &&
      requires { T{}; } &&
      requires { ::new (static_cast<void*>(nullptr)) T; };

  template< class T >
  concept move_constructible = constructible_from<T, T> && convertible_to<T, T>;

  template <class T>
  concept copy_constructible =
    move_constructible<T> &&
    constructible_from<T, T&> && convertible_to<T&, T> &&
    constructible_from<T, const T&> && convertible_to<const T&, T> &&
    constructible_from<T, const T> && convertible_to<const T, T>;
}
}

#endif

#endif
