// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_TUPLES_HH
#define DUNE_TUPLES_HH

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


  /**
   * @brief A tuple consisting of two objects.
   *
   * This is similar to std::pair
   */
  template<typename T1, typename TT>
  class Pair
  {
  public:
    /**
     * @brief The type of the first field.
     */
    typedef T1 Type1;

    /**
     * @brief The type of the first field.
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
    Pair(const Type1& t1, const T2& t2, const T3& t3,
         const T4& t4, const T5& t5, const T6& t6, const T7& t7,
         const T8& t8, const T9& t9);

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
    Pair<T1,TT>& operator=(const Pair<U1,U2>& other);

    /**
     * @brief Get the first value
     * @return The first value
     */
    Type1& first();

    /**
     * @brief Get the first value
     * @return The first value
     */
    const Type1& first() const;

    /**
     * @brief Get the second value
     * @return The first value
     */
    Type2& second();

    /**
     * @brief Get the first value
     * @return The second value
     */
    const Type2& second() const;

  private:
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
  class Pair<T1,Nil>
  {
  public:
    /**
     * @brief The type of the first field.
     */
    typedef T1 Type1;

    /**
     * @brief Constructor.
     * @param t1 The values for the first field.
     * @param t2 The value for the second field.
     */
    Pair(const Type1& first, const Nil&, const Nil&, const Nil&, const Nil&,
         const Nil&, const Nil&, const Nil&, const Nil&);

    /**
     * @brief Copy constructor for type conversion.
     */
    template<typename T2>
    Pair(const Pair<T2,Nil>& other);

    /**
     * @brief Assignment operator for type conversion.
     */
    template<typename T2>
    Pair<T1,Nil>& operator=(const Pair<T2,Nil>& other);

    /**
     * @brief Get the first value
     * @return The first value
     */
    Type1& first();

    /**
     * @brief Get the first value
     * @return The first value
     */
    const Type1& first() const;

  private:
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
   * A maximum of 9 objects is suported.
   */
  template<typename T1, typename T2 = Nil, typename T3 = Nil,
      typename T4 = Nil, typename T5 = Nil,typename T6 = Nil,
      typename T7 = Nil, typename T8 = Nil, typename T9 = Nil>
  class Tuple : public TupleToPairs<T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type
  {
  public:
    Tuple(const T1& t1=T1(), const T2& t2=T2(), const T3& t3=T3(),
          const T4& t4=T4(), const T5& t5=T5(), const T6& t6=T6(),
          const T7& t7=T7(), const T8& t8=T8(), const T9& t9=T8())
      : TupleToPairs<T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type(t1, t2, t3,
                                                       t4, t5, t6,
                                                       t7, t8, t9)
    {}

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
    static typename ElementType<N,Pair<T1,T2> >::Type& get(Pair<T1,T2>& tuple)
    {
      return Element<N-1>::get(tuple.second());
    }

    /**
     * @brief Get the N-th element of the tuple.
     * @param tuple The tuple whose N-th element we want.
     * @return The N-th element of the tuple.
     */
    template<typename T1, typename T2>
    static const typename ElementType<N,Pair<T1,T2> >::Type& get(const Pair<T1,T2>& tuple)
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
    static T1& get(Pair<T1,T2>& tuple)
    {
      return tuple.first();
    }

    /**
     * @brief Get the first element of the tuple.
     * @param tuple The tuple whose first element we want.
     * @return The first element of the tuple.
     */
    template<typename T1, typename T2>
    static const T1& get(const Pair<T1,T2>& tuple)
    {
      return tuple.first();
    }
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
    return (tuple1.first()!=tuple2.first());
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

  template<typename T1, typename TT>
  template<typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  inline Pair<T1,TT>::Pair(const Type1& first, const T2& t2, const T3& t3,
                           const T4& t4, const T5& t5, const T6& t6,
                           const T7& t7, const T8& t8, const T9& t9)
    : first_(first), second_(t2,t3,t4,t5,t6,t7,t8,t9,Nil())
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
  inline T1& Pair<T1,T2>::first()
  {
    return first_;
  }

  template<typename T1, typename T2>
  inline const T1& Pair<T1,T2>::first() const
  {
    return first_;
  }


  template<typename T1, typename T2>
  inline T2& Pair<T1,T2>::second()
  {
    return second_;
  }

  template<typename T1, typename T2>
  inline const T2& Pair<T1,T2>::second() const
  {
    return second_;
  }

  template<typename T1>
  inline Pair<T1,Nil>::Pair(const Type1& first, const Nil&, const Nil&, const Nil&, const Nil&,
                            const Nil&, const Nil&, const Nil&, const Nil&)
    : first_(first)
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
  inline T1& Pair<T1,Nil>::first()
  {
    return first_;
  }

  template<typename T1>
  inline const T1& Pair<T1,Nil>::first() const
  {
    return first_;
  }

}

#endif
