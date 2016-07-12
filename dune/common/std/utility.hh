#ifndef DUNE_COMMON_STD_UTILITY_HH
#define DUNE_COMMON_STD_UTILITY_HH

#include <cstddef>

#include <type_traits>
#include <utility>

namespace Dune
{

  namespace Std
  {


#if __cpp_lib_integer_sequence >= 201304

    using std::integer_sequence;
    using std::index_sequence;
    using std::make_integer_sequence;
    using std::make_index_sequence;
    using std::index_sequence_for;

#else // __cpp_lib_integer_sequence >= 201304

    // integer_sequence
    // ----------------

    /** \brief an implementation of std::integer_sequence as introduced in C++14
     *
     *  \tparam  T  an integer type
     *  \tparam  ...Ints  a non-type parameter pack
     */
    template< class T, T... Ints >
    class integer_sequence
    {
      static_assert( std::is_integral< T >::value, "Template parameter T is required to be an integral type" );

    public:

      /** \brief value type */
      typedef T value_type;

      /** \brief return number of elements in sequence */
      static constexpr std::size_t size () { return sizeof...( Ints ); }
    };


    /** \brief std::index_sequence as introduced in C++14
     *
     *  \tparam  ...Ints  a non-type parameter pack
     */
    template< std::size_t... Ints >
    using index_sequence = integer_sequence< std::size_t, Ints... >;

#ifndef DOXYGEN

    namespace impl {

      template<typename T, T i, T n, T... indices>
      struct _make_integer_sequence
        : public _make_integer_sequence<T,i+1,n,indices...,i>
      {};

      template<typename T, T n, T... indices>
      struct _make_integer_sequence<T,n,n,indices...>
      {
        using type = integer_sequence<T,indices...>;
      };

    }

#endif // DOXYGEN

    template<typename T, T n>
    using make_integer_sequence = typename impl::_make_integer_sequence<T,0,n>::type;

    template<std::size_t n>
    using make_index_sequence = make_integer_sequence<std::size_t,n>;

#ifndef DOXYGEN

      namespace impl {

        // This is a workaround for clang bug 14858 (https://llvm.org/bugs/show_bug.cgi?id=14858)
        // in a template alias declaration, clang always deduces sizeof...(T) as 1, if the template
        // alias is evaluated with an unpacked template parameter pack (instead of one that is explicitly
        // constructed as a list of types at the call site. This is slightly braindead (and has been around
        // since at least clang 3.0).
        // As a workaround, we lift the computation into a struct definition.
        template<typename... T>
        struct _get_pack_length
          : public std::integral_constant<std::size_t,sizeof...(T)>
        {};

      }

#endif // DOXYGEN

    template<typename... T>
    using index_sequence_for = make_index_sequence<impl::_get_pack_length<T...>{}>;

#endif // __cpp_lib_integer_sequence >= 201304


  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_UTILITY_HH
