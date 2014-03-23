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
  template<typename T1>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1> & t)
  {
    typedef tuple<T1> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2> & t)
  {
    typedef tuple<T1,T2> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3> & t)
  {
    typedef tuple<T1,T2,T3> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4> & t)
  {
    typedef tuple<T1,T2,T3,T4> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5,T6> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5,T6,T7> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5,T6,T7,T8> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9, typename T10>
  inline std::ostream& operator<<( std::ostream& os, const tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::put(os, t);
  }

  /**
   * \brief Read a tuple.
   */
  template<typename T1>
  inline std::istream& operator>>( std::istream& is, tuple<T1> & t)
  {
    typedef tuple<T1> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2> & t)
  {
    typedef tuple<T1,T2> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3> & t)
  {
    typedef tuple<T1,T2,T3> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4> & t)
  {
    typedef tuple<T1,T2,T3,T4> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5,T6> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5,T6,T7> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5,T6,T7,T8> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9, typename T10>
  inline std::istream& operator>>( std::istream& is, tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10> & t)
  {
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10> TupleT;
    return tuple_writer<tuple_size<TupleT>::value>::get(is, t);
  }

  /** }@ */
}
#endif
