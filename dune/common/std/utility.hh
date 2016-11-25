#ifndef DUNE_COMMON_STD_UTILITY_HH
#define DUNE_COMMON_STD_UTILITY_HH

#include <cstddef>

#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>

namespace Dune
{

  namespace Std
  {


#if __cpp_lib_integer_sequence >= 201304

    using std::integer_sequence;
    using std::index_sequence;
    using std::make_integer_sequence;
    using std::make_index_sequence;

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

#endif // __cpp_lib_integer_sequence >= 201304

    /**
     * \brief Create index_sequence from 0 to sizeof...(T)-1
     *
     * This should do the same as std::index_sequence_for.
     * But due to a bug in the sizeof... operator this
     * may produce wrong results with clang<3.8.
     *
     * As a workaround we provide our own implementation
     * that avoids this bug even if the std:: version
     * exists.
     *
     * This implemenation can be dropped, once we require
     * a minimum clang version that has this bug fixed (i.e. >=3.8).
     */
    template<typename... T>
    using index_sequence_for = make_index_sequence<typename Dune::SizeOf<T...>{}>;



  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_UTILITY_HH
