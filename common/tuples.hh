// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_TUPLES_HH
#define DUNE_TUPLES_HH

#include <ostream>
#include "typetraits.hh"
#include "helpertemplates.hh"

namespace Dune
{
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief Contains classes that implement tuples.
   *
   * This a refined implementation of the approach defined in
   * in the article &quot;Tuples and multiple return values
   * in C++&quot; of Jaakko J&auml;rvi (Turku Centre of Computer
   * Science, TUCS Technical Report No 249, March 1999,
   * ISBN 952-12-0401-X, ISSN 1239-1891) available from the
   * <a href="http://www.tucs.fi/publications/">TUCS publications archive</a>
   * @author Markus Blatt
   */

  /**
   * @brief An empty class.
   */
  struct Nil
  {};

  namespace
  {
    inline const Nil nullType()
    {
      return Nil();
    }
  }

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
    typedef typename ConstantVolatileTraits<T>::ConstType& ConstType;
    typedef T& NonConstType;
    typedef T& ParameterType;
  };

  /**
   * @brief A tuple consisting of two objects.
   *
   * This is similar to std::pair
   */
  template<typename T1, typename TT>
  struct Pair
  {
    /**
     * @brief The type of the first field.
     */
    typedef T1 Type1;

    /**
     * @brief The type of the second field.
     */
    typedef TT Type2;
    //     enum{
    //     /**
    //      * @brief The number of values we hold.
    //      */
    //       values = 2;
    //     };

    /**
     * @brief Constructor
     *
     * @param t1 The value of the first field.
     * @param t2 The value of the second field.
     */
    template<typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    Pair(typename TupleAccessTraits<T1>::ParameterType t1, T2& t2, T3& t3, T4& t4, T5& t5,
         T6& t6, T7& t7, T8& t8, T9& t9);

    /**
     * @brief Constructor
     *
     * @param t1 The value of the first field.
     * @param t2 The value of the second field.
     */
    Pair(typename TupleAccessTraits<Type1>::ParameterType t1, TT& t2);

    Pair();

    /**
     * @brief Copy Constructor for implicit type conversion
     * @param other The tuple to copy.
     */
    template<typename U1, typename U2>
    Pair(const Pair<U1,U2>& other);

    /**
     * @brief Assignment operator for implicit type conversion
     * @param other The tuple to assign.
     */
    template<typename U1, typename U2>
    Pair& operator=(const Pair<U1,U2>& other);

    Pair& operator=(const Pair& other);

    /**
     * @brief Get the first value
     * @return The first value
     */
    typename TupleAccessTraits<Type1>::NonConstType first();

    /**
     * @brief Get the first value
     * @return The first value
     */
    typename TupleAccessTraits<Type1>::ConstType
    first() const;

    /**
     * @brief Get the second value
     * @return The second value
     */
    typename TupleAccessTraits<Type2>::NonConstType
    second();

    /**
     * @brief Get the second value
     * @return The second value
     */
    typename TupleAccessTraits<Type2>::ConstType
    second() const;

    /** @brief The value of the first field. */
    Type1 first_;
    /** @brief The value of the second field. */
    Type2 second_;

  };

  /**
   * @brief A tuple consisting of one object.
   * Specialization of Pair that really is a single value.
   */
  template<typename T1>
  struct Pair<T1,Nil>
  {
    /**
     * @brief The type of the first field.
     */
    typedef T1 Type1;

    /**
     * @brief The type of the (non-existent) second field is Nil.
     * This typedef is useful in template metaprogramming, since it allows
     * you to specialise for Nil instead of Pair<T, Nil>
     */
    typedef Nil Type2;

    /**
     * @brief Constructor.
     * @param t1 The values for the first field.
     * @param t2 The value for the second field.
     */
    Pair(typename TupleAccessTraits<T1>::ParameterType first, const Nil&, const Nil&, const Nil&, const Nil&,
         const Nil&, const Nil&, const Nil&, const Nil&);

    /**
     * @brief Constructor.
     * @param t1 The values for the first field.
     * @param t2 The value for the second field.
     */
    Pair(typename TupleAccessTraits<T1>::ParameterType first,
         const Nil&);

    Pair();

    /**
     * @brief Copy constructor for type conversion.
     */
    template<typename T2>
    Pair(const Pair<T2,Nil>& other);

    /**
     * @brief Assignment operator for type conversion.
     */
    template<typename T2>
    Pair& operator=(const Pair<T2,Nil>& other);

    /**
     * @brief Assignment operator.
     */
    Pair& operator=(const Pair& other);

    /**
     * @brief Get the first value
     * @return The first value
     */
    typename TupleAccessTraits<Type1>::NonConstType
    first();

    /**
     * @brief Get the first value
     * @return The first value
     */
    typename TupleAccessTraits<Type1>::ConstType
    first() const;

    /** @brief The value of the first field.*/
    Type1 first_;
  };


  /**
   * @brief Converts the Tuple to a list of pairs.
   */
  template<typename T1, typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  struct TupleToPairs
  {
    typedef Pair<T1, typename TupleToPairs<T2,T3,T4,T5,T6,T7,T8,T9,Nil>::Type > Type;
  };

  /**
   * @brief Specialization for a tuple consisting only of one type.
   */
  template<typename T1>
  struct TupleToPairs<T1,Nil,Nil,Nil,Nil,Nil,Nil,Nil,Nil>
  {
    typedef Pair<T1,Nil> Type;
  };

  /**
   * @brief A Tuple of objects.
   *
   * A maximum of 9 objects is supported.
   *
   * Use the following construction to access the individual elements.
     \code
      Tuple<std::string, float*, int> my_tuple;

      std:string& s = Element<0>::get(my_tuple);
      float*      p = Element<1>::get(my_tuple);

      // Access the third element in a generic way
      typedef ElementType<2, Tuple<std::string, float*, int> >::Type Type;
      Type&       i = Element<2>::get(my_tuple);
     \endcode
   */
  template<typename T1, typename T2 = Nil, typename T3 = Nil,
      typename T4 = Nil, typename T5 = Nil,typename T6 = Nil,
      typename T7 = Nil, typename T8 = Nil, typename T9 = Nil>
  class Tuple : public TupleToPairs<T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type
  {
  public:
    //! Type of the first Pair defining the Tuple
    typedef typename TupleToPairs<T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type FirstPair;

    Tuple()
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1)
      : FirstPair(t1, nullType(), nullType(), nullType(),
                  nullType(), nullType(), nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2)
      : FirstPair(t1, t2, nullType(), nullType(),
                  nullType(), nullType(), nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3)
      : FirstPair(t1, t2, t3, nullType(),
                  nullType(), nullType(), nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4)
      : FirstPair(t1, t2, t3, t4,
                  nullType(), nullType(), nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4,
          typename TupleAccessTraits<T5>::ParameterType t5)
      : FirstPair(t1, t2, t3, t4,
                  t5, nullType(), nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4,
          typename TupleAccessTraits<T5>::ParameterType t5,
          typename TupleAccessTraits<T6>::ParameterType t6)
      : FirstPair(t1, t2, t3, t4,
                  t5, t6, nullType(), nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4,
          typename TupleAccessTraits<T5>::ParameterType t5,
          typename TupleAccessTraits<T6>::ParameterType t6,
          typename TupleAccessTraits<T7>::ParameterType t7)
      : FirstPair(t1, t2, t3, t4,
                  t5, t6, t7, nullType(),
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4,
          typename TupleAccessTraits<T5>::ParameterType t5,
          typename TupleAccessTraits<T6>::ParameterType t6,
          typename TupleAccessTraits<T7>::ParameterType t7,
          typename TupleAccessTraits<T8>::ParameterType t8)
      : FirstPair(t1, t2, t3, t4,
                  t5, t6, t7, t8,
                  nullType())
    {}

    Tuple(typename TupleAccessTraits<T1>::ParameterType t1,
          typename TupleAccessTraits<T2>::ParameterType t2,
          typename TupleAccessTraits<T3>::ParameterType t3,
          typename TupleAccessTraits<T4>::ParameterType t4,
          typename TupleAccessTraits<T5>::ParameterType t5,
          typename TupleAccessTraits<T6>::ParameterType t6,
          typename TupleAccessTraits<T7>::ParameterType t7,
          typename TupleAccessTraits<T8>::ParameterType t8,
          typename TupleAccessTraits<T9>::ParameterType t9)
      : FirstPair(t1, t2, t3, t4, t5, t6, t7, t8, t9)
    {}

    template<class U1, class U2>
    Tuple& operator=(const Pair<U1,U2>& other)
    {
      FirstPair::operator=(other);
      return *this;
    }
  };

  /**
   * @brief Get the type of the N-th element of the tuple.
   */
  template<int N, class Tuple>
  struct ElementType;

  template<int N, typename T1, typename T2>
  struct ElementType<N,Pair<T1,T2> >
  {
    /**
     * @brief The type of the N-th element of the tuple.
     */
    typedef typename ElementType<N-1,T2>::Type Type;
  };

  /**
   * @brief Get the type of the first element of the tuple.
   */
  template<typename T1, typename T2>
  struct ElementType<0, Pair<T1,T2> >
  {
    /**
     * @brief The type of the first element of the tuple.
     */
    typedef T1 Type;
  };


  /**
   * @brief Get the N-th element of a tuple.
   */
  template<int N>
  struct Element
  {
    /**
     * @brief Get the N-th element of the tuple.
     * @param tuple The tuple whose N-th element we want.
     * @return The N-th element of the tuple.
     */
    template<typename T1, typename T2>
    static typename TupleAccessTraits<
        typename ElementType<N,Pair<T1,T2> >::Type
        >::NonConstType
    get(Pair<T1,T2>& tuple)
    {
      return Element<N-1>::get(tuple.second());
    }

    /**
     * @brief Get the N-th element of the tuple.
     * @param tuple The tuple whose N-th element we want.
     * @return The N-th element of the tuple.
     */
    template<typename T1, typename T2>
    static typename TupleAccessTraits<
        typename ElementType<N,Pair<T1,T2> >::Type
        >::ConstType
    get(const Pair<T1,T2>& tuple)
    {
      return Element<N-1>::get(tuple.second());
    }
  };

  /**
   * @brief Get the first element of a tuple.
   */
  template<>
  struct Element<0>
  {
    /**
     * @brief Get the first element of the tuple.
     * @param tuple The tuple whose first element we want.
     * @return The first element of the tuple.
     */
    template<typename T1, typename T2>
    static typename TupleAccessTraits<T1>::NonConstType get(Pair<T1,T2>& tuple)
    {
      return tuple.first();
    }

    /**
     * @brief Get the first element of the tuple.
     * @param tuple The tuple whose first element we want.
     * @return The first element of the tuple.
     */
    template<typename T1, typename T2>
    static typename TupleAccessTraits<T1>::ConstType get(const Pair<T1,T2>& tuple)
    {
      return tuple.first();
    }
  };

  /**
   * @brief Template meta_programm to query the size of a tuple
   *
   */
  template<typename T>
  struct Size
  {
    enum {
      /** @brief The value of the size of the tuple */
      value = -100000
    };
  };

  template<typename T1, typename T2>
  struct Size<Pair<T1,T2> >
  {
    enum { value=1+Size<T2>::value};
  };


  template<typename T1>
  struct Size<Pair<T1,Nil> >
  {
    enum { value=1};
  };


  /**
   * @brief Equality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1, typename T2, typename U1, typename U2>
  inline bool operator==(const Pair<T1,T2>& tuple1, const Pair<U1,U2>& tuple2)
  {
    return (tuple1.first()==tuple2.first() && tuple1.second()==tuple2.second());
  }

  /**
   * @brief Inequality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1, typename T2, typename U1, typename U2>
  inline bool operator!=(const Pair<T1,T2>& tuple1, const Pair<U1,U2>& tuple2)
  {
    return (tuple1.first()!=tuple2.first() || tuple1.second()!=tuple2.second());
  }

  /**
   * @brief Less operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1, typename T2, typename U1, typename U2>
  inline bool operator<(const Pair<T1,T2>& tuple1, const Pair<U1,U2>& tuple2)
  {
    return tuple1.first() < tuple2.first()
           || (tuple1.first() == tuple2.first() && tuple1.second() < tuple2.second());
  }

  /**
   * @brief Equality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1,typename U1>
  inline bool operator==(const Pair<T1,Nil>& tuple1, const Pair<U1,Nil>& tuple2)
  {
    return (tuple1.first()==tuple2.first());
  }

  /**
   * @brief Inequality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1, typename U1>
  inline bool operator!=(const Pair<T1,Nil>& tuple1, const Pair<U1,Nil>& tuple2)
  {
    IsTrue<IsInteroperable<T1,U1>::value>::yes();
    return (tuple1.first()!=tuple2.first());
  }

  /**
   * @brief Less operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple,
   */
  template<typename T1, typename U1>
  inline bool operator<(const Pair<T1,Nil>& tuple1, const Pair<U1,Nil>& tuple2)
  {
    return (tuple1.first()<tuple2.first());
  }

  /**
   * @brief Equality comparison operator for tuples.
   *
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple.
   * @return False as the type of the compared objects are different.
   */
  template<typename T1,typename U1, typename U2>
  inline bool operator==(const Pair<T1,Nil>& tuple1, const Pair<U1,U2>& tuple2)
  {
    return false;
  }

  /**
   * @brief Inequality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple.
   * @return True as the type of the compared objects are different.
   */
  template<typename T1, typename U1, typename U2>
  inline bool operator!=(const Pair<T1,Nil>& tuple1, const Pair<U1,U2>& tuple2)
  {
    return true;
  }


  /**
   * @brief Equality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple.
   * @return False as the type of the compared objects are different.
   */
  template<typename T1, typename T2, typename U1>
  inline bool operator==(const Pair<T1,T2>& tuple1, const Pair<U1,Nil>& tuple2)
  {
    return false;
  }

  /**
   * @brief Inequality comparison operator for tuples.
   * @param tuple1 The first tuple.
   * @param tuple2 The second tuple.
   * @return True as the type of the compared objects are different.
   */
  template<typename T1, typename T2, typename U1>
  inline bool operator!=(const Pair<T1,T2>& tuple1, const Pair<U1,Nil>& tuple2)
  {
    return true;
  }

  /**
   * @brief Create a tuple and initialize it.
   * @param first The value of the first field.
   * @param second The value of the second field.
   */
  template<typename T1, typename T2>
  inline Pair<T1,T2> makePair(const T1& first, const T2& second)
  {
    return Pair<T1,T2>(first, second);
  }

  /**
   * @brief Print a pair or tuple.
   */
  template<typename T1, typename T2>
  inline std::ostream& operator<<(std::ostream& os, const Pair<T1,T2>& pair)
  {
    os<<pair.first()<<" "<<pair.second();
    return os;
  }

  template<typename T1>
  inline std::ostream& operator<<(std::ostream& os, const Pair<T1,Nil>& pair)
  {
    os<<pair.first();
    return os;
  }

  template<class T1>
  inline Tuple<T1&> tie(T1& t1) {
    return Tuple<T1&> (t1);
  }

  template<class T1, class T2>
  inline Tuple<T1&, T2&> tie(T1& t1, T2& t2) {
    return Tuple<T1&, T2&> (t1, t2);
  }

  template<class T1, class T2, class T3>
  inline Tuple<T1&, T2&, T3&> tie(T1& t1, T2& t2, T3& t3) {
    return Tuple<T1&, T2&, T3&> (t1, t2, t3);
  }

  template<class T1, class T2, class T3, class T4>
  inline Tuple<T1&, T2&, T3&, T4&> tie(T1& t1, T2& t2, T3& t3, T4& t4) {
    return Tuple<T1&, T2&, T3&, T4&> (t1, t2, t3, t4);
  }

  template<class T1, class T2, class T3, class T4, class T5>
  inline Tuple<T1&, T2&, T3&, T4&, T5&>
  tie(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) {
    return Tuple<T1&, T2&, T3&, T4&, T5&> (t1, t2, t3, t4, t5);
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  inline Tuple<T1&, T2&, T3&, T4&, T5&, T6&>
  tie(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6) {
    return Tuple<T1&, T2&, T3&, T4&, T5&, T6&> (t1, t2, t3, t4, t5, t6);
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  inline Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&>
  tie(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7) {
    return Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&> (t1, t2, t3, t4, t5, t6, t7);
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7,
      class T8>
  inline Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&>
  tie(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8) {
    return Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&>
             (t1, t2, t3, t4, t5, t6, t7, t8);
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6, class T7,
      class T8, class T9>
  inline Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&>
  tie(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8, T9& t9) {
    return Tuple<T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&>
             (t1, t2, t3, t4, t5, t6, t7, t8, t9);
  }


  template<typename T1, typename TT>
  template<typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  inline Pair<T1,TT>::Pair(typename TupleAccessTraits<T1>::ParameterType first,
                           T2& t2, T3& t3, T4& t4, T5& t5,
                           T6& t6, T7& t7, T8& t8, T9& t9)
    : first_(first), second_(t2,t3,t4,t5,t6,t7,t8,t9, nullType())
  {}

  template <typename T1, typename TT>
  inline Pair<T1, TT>::Pair(typename TupleAccessTraits<T1>::ParameterType first, TT& second)
    : first_(first), second_(second)
  {}

  template<typename T1, typename T2>
  inline Pair<T1,T2>::Pair()
    : first_(), second_()
  {}

  template<typename T1, typename T2>
  template<typename U1, typename U2>
  inline Pair<T1,T2>::Pair(const Pair<U1,U2>& other)
    : first_(other.first_), second_(other.second_)
  {}

  template<typename T1, typename T2>
  template<typename U1, typename U2>
  inline Pair<T1,T2>& Pair<T1,T2>::operator=(const Pair<U1,U2>& other)
  {
    first_=other.first_;
    second_=other.second_;
    return *this;
  }

  template<typename T1, typename T2>
  inline Pair<T1,T2>& Pair<T1,T2>::operator=(const Pair& other)
  {
    first_=other.first_;
    second_=other.second_;
    return *this;
  }

  template<typename T1, typename T2>
  inline typename TupleAccessTraits<T1>::NonConstType
  Pair<T1,T2>::first()
  {
    return first_;
  }

  template<typename T1, typename T2>
  inline typename TupleAccessTraits<T1>::ConstType
  Pair<T1,T2>::first() const
  {
    return first_;
  }


  template<typename T1, typename T2>
  inline typename TupleAccessTraits<T2>::NonConstType
  Pair<T1,T2>::second()
  {
    return second_;
  }

  template<typename T1, typename T2>
  inline typename TupleAccessTraits<T2>::ConstType
  Pair<T1,T2>::second() const
  {
    return second_;
  }

  template<typename T1>
  inline Pair<T1,Nil>::Pair(typename TupleAccessTraits<T1>::ParameterType first,
                            const Nil&, const Nil&, const Nil&, const Nil&,
                            const Nil&, const Nil&, const Nil&, const Nil&)
    : first_(first)
  {}

  template <typename T1>
  inline Pair<T1, Nil>::Pair(typename TupleAccessTraits<T1>::ParameterType first,
                             const Nil&)
    : first_(first)
  {}

  template<typename T1>
  inline Pair<T1,Nil>::Pair()
    : first_()
  {}

  template<typename T1>
  template<typename T2>
  inline Pair<T1,Nil>::Pair(const Pair<T2,Nil>& other)
    : first_(other.first_)
  {}

  template<typename T1>
  template<typename T2>
  Pair<T1,Nil>& Pair<T1,Nil>::operator=(const Pair<T2,Nil>& other)
  {
    first_ = other.first_;
    return *this;
  }


  template<typename T1>
  Pair<T1,Nil>& Pair<T1,Nil>::operator=(const Pair& other)
  {
    first_ = other.first_;
    return *this;
  }

  template<typename T1>
  inline typename TupleAccessTraits<T1>::NonConstType
  Pair<T1,Nil>::first()
  {
    return first_;
  }

  template<typename T1>
  inline typename TupleAccessTraits<T1>::ConstType
  Pair<T1,Nil>::first() const
  {
    return first_;
  }

}

#endif
