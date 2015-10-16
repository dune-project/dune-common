#ifndef DUNE_COMMON_STD_UTILITY_HH
#define DUNE_COMMON_STD_UTILITY_HH

#include <cstddef>

#include <type_traits>
#include <utility>

#include <dune/common/std/constexpr.hh>
#include <dune/common/std/noexcept.hh>

namespace Dune
{

  namespace Std
  {

    // integer_sequence
    // ----------------

    /** \brief an implementation of std::integer_sequence to be introduced in
     *         C++14
     *
     *  \tparam  T  an integer type
     *  \tparam  ...Ints  a non-type parameter pack
     */
    template< class T, T... Ints >
    class integer_sequence
    {
      static_assert( std::is_integral< T >::value, "Template parameter T is required to be an integral type" );

    public:
#ifndef DOXYGEN

      template< class U >
      struct rebind
      {
        typedef integer_sequence< U, static_cast< U >( Ints )... > type;
      };

#endif // #ifndef DOXYGEN

      /** \brief value type */
      typedef T value_type;

      /** \brief return number of elements in sequence */
      static DUNE_CONSTEXPR std::size_t size () { return sizeof...( Ints ); }
    };



    // index_sequence
    // --------------
    // While this is similar to std::index_sequence, there is one
    // major problem with the present implementation. Our index_sequence
    // is only derived from integer_sequence but not equal to one.
    // As a consequence template specializations that are common
    // in nontrivial use cases will fail unless we take special
    // care about index_sequence. The correct implementation is
    // a template alias. We should switch to the correct implementation
    // (i.e. a template alias) as soon as we allow this c++11 feature
    // in dune-common.

    /** \brief a function similar to std::index_sequence to be introduced in
     *         C++14
     *
     *  \tparam  ...Ints  a non-type parameter pack
     */
    template< std::size_t... Ints >
    class index_sequence
      : public integer_sequence< std::size_t, Ints... >
    {};



#ifndef DOXYGEN

    // make_index_sequence_impl
    // ------------------------

    template< std::size_t N >
    class make_index_sequence_impl;

    template<>
    class make_index_sequence_impl< 0u >
    {
    public:
      typedef index_sequence<> type;
    };

    template<>
    class make_index_sequence_impl< 1u >
    {
    public:
      typedef index_sequence< 0 > type;
    };

    template< std::size_t N >
    class make_index_sequence_impl
    {
      static_assert( N >= 0, "Cannot produce an index sequence of negative length" );

      template< std::size_t... I1, std::size_t... I2 >
      static index_sequence< I1..., (sizeof...( I1 )+I2)... >
      add ( index_sequence< I1... >, index_sequence< I2... > )
      {
        return index_sequence< I1..., (sizeof...( I1 )+I2)... >();
      }

    public:
      typedef decltype( add( typename make_index_sequence_impl< N/2 >::type(), typename make_index_sequence_impl< N-N/2 >::type() ) ) type;
    };

#endif // #ifndef DOXYGEN



    // make_index_sequence
    // -------------------

    /** \fn make_index_sequence
     *
     *  \brief a function similar to std::make_index_sequence to be introduced
     *         in C++14
     *
     *  \tparam  N  requested size of index sequence
     */
    template< std::size_t N >
    static DUNE_CONSTEXPR inline typename make_index_sequence_impl< N >::type make_index_sequence ()
    {
      return typename make_index_sequence_impl< N >::type();
    }



    // make_integer_sequence
    // ---------------------

    /** \fn make_integer_sequence
     *
     *  \brief a function similar to std::make_integer_sequence to be
     *         introduced in C++14
     *
     *  \tparam  T  an integer type
     *  \tparam  N  requested size of integer sequence
     */
    template< class T, T N >
    static DUNE_CONSTEXPR inline typename make_index_sequence_impl< N >::type::template rebind< T >::type
    make_integer_sequence ()
    {
      return typename make_index_sequence_impl< N >::type::template rebind< T >::type();
    }



    // index_sequence_for
    // ------------------

    /** \fn index_sequence_for
     *
     *  \brief a function similar to std::index_sequence_for to be introduced
     *         in C++14
     *
     *  \tparam  ...T  a type parameter pack
     */
    template< class... T >
    static DUNE_CONSTEXPR inline typename make_index_sequence_impl< sizeof...( T ) >::type
    index_sequence_for ()
    {
      return typename make_index_sequence_impl< sizeof...( T ) >::type();
    }

    using std::declval;

  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_UTILITY_HH
