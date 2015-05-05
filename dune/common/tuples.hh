// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TUPLES_HH
#define DUNE_TUPLES_HH

#include <iostream>

#include "typetraits.hh"
#include "unused.hh"

#include <tuple>

namespace Dune {
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief Fallback implementation of the std::tuple class
   *
   * This a refined implementation of the approach defined in
   * in the article &quot;Tuples and multiple return values
   * in C++&quot; of Jaakko J&auml;rvi (Turku Centre of Computer
   * Science, TUCS Technical Report No 249, March 1999,
   * ISBN 952-12-0401-X, ISSN 1239-1891) available from the
   * <a href="http://www.tucs.fi/publications/">TUCS publications archive</a>
   * @author Markus Blatt
   */


  template<class T>
  struct TupleAccessTraits
  {
    typedef typename ConstantVolatileTraits<T>::ConstType& ConstType;
    typedef T& NonConstType;
    typedef const typename ConstantVolatileTraits<T>::UnqualifiedType& ParameterType;
  };

  template<class T>
  struct TupleAccessTraits<T*>
  {
    typedef typename ConstantVolatileTraits<T>::ConstType* ConstType;
    typedef T* NonConstType;
    typedef T* ParameterType;
  };

  template<class T>
  struct TupleAccessTraits<T&>
  {
    typedef T& ConstType;
    typedef T& NonConstType;
    typedef T& ParameterType;
  };

  // pull in default implementations
  using std::tuple;
  using std::tuple_element;
  using std::get;
  using std::tuple_size;
  using std::tie;
  using std::make_tuple;


  template<int i>
  struct tuple_writer
  {
    template<class T>
    static std::ostream& put(std::ostream& os, const T& t, const char* delim=", ")
    {
      return tuple_writer<i-1>::put(os,t,delim)<<delim<<Dune::get<i-1>(t);
    }

    template< class T >
    static std::istream &get ( std::istream &is, T &t, const char *delim = "," )
    {
      tuple_writer< i-1 >::get( is, t, delim );
      for( const char *it = delim; is && (*it != 0); ++it )
      {
        char c = 0;
        is >> c;
        if( c != *it )
          is.setstate( std::ios::failbit );
      }
      return is >> Dune::get< i-1 >( t );
    }
  };

  template<>
  struct tuple_writer<1>
  {
    template<class T>
    static std::ostream& put(std::ostream& os, const T& t, const char* delim=", ")
    {
      DUNE_UNUSED_PARAMETER(delim);
      return os<<Dune::get<0>(t);
    }

    template< class T >
    static std::istream &get ( std::istream &is, T &t, const char *delim = ", " )
    {
      return is >> Dune::get< 0 >( t );
    }
  };

  template<>
  struct tuple_writer<0>
  {
    template<class T>
    static std::ostream& put(std::ostream& os, const T& t, const char* delim=", ")
    {
      return os;
    }

    template< class T >
    static std::istream &get ( std::istream &is, T &t, const char *delim = ", " )
    {
      return is;
    }
  };

  /**
   * \brief Print a tuple.
   */
  template<typename... Ts>
  inline std::ostream& operator<<(std::ostream& os, const tuple<Ts...>& t)
  {
    return tuple_writer<sizeof...(Ts)>::put(os, t);
  }

  /**
   * \brief Read a tuple.
   */
  template<typename... Ts>
  inline std::istream& operator>>(std::istream& is, tuple<Ts...>& t)
  {
    return tuple_writer<sizeof...(Ts)>::get(is, t);
  }

  /** }@ */
}
#endif
