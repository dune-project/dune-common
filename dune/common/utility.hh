// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UTILITY_HH
#define DUNE_UTILITY_HH

#include "tuples.hh"

namespace Dune {

  /** @ addtogroup Common
   *
   * @{
   */

  /**
   * @file
   * @brief Contain utility classes which can be used with tuples.
   */

  /**
   * @brief A helper template that initializes a tuple consisting of pointers
   * to NULL.
   *
   * A tuple of NULL pointers may be useful when you use a tuple of pointers
   * in a class which you can only initialise in a later stage.
   */
  template <class Tuple>
  class NullPointerInitialiser {};

  namespace
  {
    /**
     * @brief Helper class for initialising null pointers.
     *
     * First template parameter is the tuple.
     * Second templates parameter is the size of the tuple.
     */
    template<class T, int size>
    struct InitialiserHelper
    {};

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2,T3,T4,T5,T6,T7,T8,T9>,1>
    {
      static inline tuple<T1*,T2,T3,T4,T5,T6,T7,T8,T9> apply()
      {
        return tuple<T1*,T2,T3,T4,T5,T6,T7,T8,T9>(0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3,T4,T5,T6,T7,T8,T9>,2>
    {
      static inline tuple<T1*,T2*,T3,T4,T5,T6,T7,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3,T4,T5,T6,T7,T8,T9>(0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4,T5,T6,T7,T8,T9>,3>
    {
      static inline tuple<T1*,T2*,T3*,T4,T5,T6,T7,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4,T5,T6,T7,T8,T9>(0,0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5,T6,T7,T8,T9>,4>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5,T6,T7,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5,T6,T7,T8,T9>(0,0,0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5*,T6,T7,T8,T9>,5>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5*,T6,T7,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5*,T6,T7,T8,T9>(0,0,0,0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7,T8,T9>,6>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7,T8,T9>(0,0,0,0,0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8,T9>,7>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8,T9>(0,0,0,0,0,0,0);
      }
    };
    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9>,8>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9>(0,0,0,0,0,0,0,0);
      }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct InitialiserHelper<tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9*>,9>
    {
      static inline tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9*> apply()
      {
        return tuple<T1*,T2*,T3*,T4*,T5*,T6*,T7*,T8*,T9*>(0,0,0,0,0,0,0,0,0);
      }
    };
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  class NullPointerInitialiser<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >
  {
  public:
    typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> ResultType;

    static inline ResultType apply()
    {
      return InitialiserHelper<ResultType, tuple_size<ResultType>::value>::apply();
    }
  };

  /**
   * @brief Deletes all objects pointed to in a tuple of pointers.
   *
   * \warning Pointers cannot be set to NULL, so calling the Deletor twice
   * or accessing elements of a deleted tuple leads to unforeseeable results!
   */
  template <class Tuple>
  struct PointerPairDeletor {};

  namespace
  {

    template<class T, int s>
    struct PointerDeletor
    {};

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9, int s>
    struct PointerDeletor<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,s>
    {
      static void apply(tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>& t)
      {

        PointerDeletor<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,s-1>::apply(t);
        delete get<s-1>(t);
      }
    };
    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct PointerDeletor<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,0>
    {
      static void apply(tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>& t)
      {}
    };
  }

  template<typename T1, typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  struct PointerPairDeletor<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >
  {
    static void apply(tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>& t)
    {
      PointerDeletor<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,
          tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value>
      ::apply(t);
    }

  };

  /**
   * @brief Helper template to calculate length of a tuple.
   */
  template <class Tuple>
  struct Length {};

  template<typename T1, typename T2, typename T3, typename T4, typename T5,
      typename T6, typename T7, typename T8, typename T9>
  struct Length<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >
  {
    enum {
      value=tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value
    };
  };

  namespace
  {
    /**
     * @brief Helper class for getting a converted tuple.
     *
     * Eval describes the conversion policy, T is the tuple type
     * and s is the size of the (sub) tuple, to process.
     */
    template<template <class> class Eval, typename T, int s>
    struct ForEachTypeHelper
    {};

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,0>
    {
      typedef tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> Type;
    };


    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,1>
    {
      typedef tuple<typename Eval<T1>::Type,T2,T3,T4,T5,T6,T7,T8,T9> Type;
    };

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,2>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,T3,T4,T5,T6,T7,T8,T9> Type;
    };

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,3>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          T4,T5,T6,T7,T8,T9> Type;
    };

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,4>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,T5,T6,T7,T8,T9> Type;
    };


    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,5>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,typename Eval<T5>::Type,T6,T7,T8,T9> Type;
    };


    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,6>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,typename Eval<T5>::Type,typename Eval<T6>::Type,
          T7,T8,T9> Type;
    };

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,7>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,typename Eval<T5>::Type,typename Eval<T6>::Type,
          typename Eval<T7>::Type,T8,T9> Type;
    };


    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,8>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,typename Eval<T5>::Type,typename Eval<T6>::Type,
          typename Eval<T7>::Type,typename Eval<T8>::Type,T9> Type;
    };

    template<template <class> class Eval, typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    struct ForEachTypeHelper<Eval,tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>,9>
    {
      typedef tuple<typename Eval<T1>::Type,typename Eval<T2>::Type,typename Eval<T3>::Type,
          typename Eval<T4>::Type,typename Eval<T5>::Type,typename Eval<T6>::Type,
          typename Eval<T7>::Type,typename Eval<T8>::Type,typename Eval<T9>::Type> Type;
    };
  }


  /**
   * @brief Helper template to clone the type definition of a tuple with the
   * storage types replaced by a user-defined rule.
   *
   * Suppose all storage types A_i in a tuple define a type A_i::B. You can
   * build up a pair consisting of the types defined by A_i::B in the following
   * way:
     \code
     template <class A>
     struct MyEvaluator {
     typedef typename A::B Type;
     };

     typedef ForEachType<MyEvaluator, ATuple>::Type BTuple;
     \endcode
   * Here, MyEvaluator is a helper struct that extracts the correct type from
   * the storage types of the tuple defined by the tuple ATuple.
   */
  template <template <class> class TypeEvaluator, class TupleType>
  struct ForEachType {
    typedef typename ForEachTypeHelper<TypeEvaluator,TupleType,
        tuple_size<TupleType>::value>::Type Type;

  };

  namespace
  {
    template<int i, typename T1,typename F>
    struct Visitor
    {
      static inline void visit(F& func, T1& t1)
      {
        func.visit(get<tuple_size<T1>::value-i>(t1));
        Visitor<i-1,T1,F>::visit(func, t1);
      }
    };

    template<typename T1,typename F>
    struct Visitor<0,T1,F>
    {
      static inline void visit(F& func, T1& t1)
      {}
    };

    template<int i, typename T1, typename T2,typename F>
    struct PairVisitor
    {
      static inline void visit(F& func, T1& t1, T2& t2)
      {
        func.visit(get<tuple_size<T1>::value-i>(t1), get<tuple_size<T2>::value-i>(t2));
        PairVisitor<i-1,T1,T2,F>::visit(func, t1, t2);
      }
    };

    template<typename T1, typename T2, typename F>
    struct PairVisitor<0,T1,T2,F>
    {
      static inline void visit(F& func, T1& t1, T2& t2)
      {}
    };
  }

  /**
   * @brief Helper template which implements iteration over all storage
   * elements in a tuple.
   *
   * Compile-time constructs that allows to process all elements in a tuple.
   * The exact operation performed on an element is defined by a function
   * object, which needs to implement a visit method which is applicable to
   * all storage elements of a tuple.
   *
   * The following example implements a function object which counts the
   * elements in a tuple
     \code
     template <class T>
     struct Counter {
     Counter() : result_(0) {}

     template <class T>
     void visit(T& elem) { ++result_; }

     int result_;
     };
     \endcode
   * The number of elements in the tuple are stored in the member variable
   * result_. The Counter can be used as follows, assuming a tuple t of type
   * MyTuple is given:
     \code
     Counter c;
     ForEachValue<MyTuple> forEach(t);

     forEach.apply(c);
     std::cout << "Number of elements is: " << c.result_ << std::endl;
     \endcode
   */
  template <class TupleType>
  class ForEachValue {
  public:
    //! \brief Constructor
    //! \param tuple The tuple which we want to process.
    ForEachValue(TupleType& tuple) : tuple_(tuple) {}

    //! \brief Applies a function object to each storage element of the tuple.
    //! \param f Function object.
    template <class Functor>
    void apply(Functor& f) {
      Visitor<tuple_size<TupleType>::value,TupleType,Functor>::visit(f, tuple_);
    }
  private:
    TupleType& tuple_;
  };

  //- Definition ForEachValuePair class
  // Assertion: both tuples have the same length and the contained types are
  // compatible in the sense of the applied function object
  /**
   * @brief Extension of ForEachValue to two tuples...
   *
   * This class provides the framework to process two tuples at once. It works
   * the same as ForEachValue, just that the corresponding function object
   * takes one argument from the first tuple and one argument from the second.
   *
   * \note You have to ensure that the two tuples you provide are compatible
   * in the sense that they have the same length and that the objects passed
   * to the function objects are related in meaningful way. The best way to
   * enforce it is to build the second tuple from the existing first tuple
   * using ForEachType.
   */
  template <class TupleType1, class TupleType2>
  class ForEachValuePair {
  public:
    //! Constructor
    //! \param t1 First tuple.
    //! \param t2 Second tuple.
    ForEachValuePair(TupleType1& t1, TupleType2& t2) :
      tuple1_(t1),
      tuple2_(t2)
    {}

    //! Applies the function object f to the pair of tuples.
    //! \param f The function object to apply on the pair of tuples.
    template <class Functor>
    void apply(Functor& f) {
      PairVisitor<tuple_size<TupleType1>::value,TupleType1,TupleType2,Functor>
      ::visit(f, tuple1_, tuple2_);
    }
  private:
    TupleType1& tuple1_;
    TupleType2& tuple2_;
  };

  //- Reverse element access
  /**
   * @brief Type for reverse element access.
   *
   * Counterpart to ElementType for reverse element access.
   */
  template <int N, class Tuple>
  struct AtType {
    typename ElementType<Length<Tuple>::value - N - 1,
        Tuple>::Type Type;
  };

  /**
   * @brief Reverse element access.
   *
   * While Element<...> gives you the arguments beginning at the front of a
   * tuple, At<...> starts at the end, which may be more convenient, depending
   * on how you built your tuple.
   */
  template <int N>
  struct At
  {

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    static typename TupleAccessTraits<
        typename tuple_element<tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value - N - 1,
            tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::type>::NonConstType
    get(tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>& t)
    {
      return Dune::get<tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value - N - 1>(t);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
        typename T6, typename T7, typename T8, typename T9>
    static typename TupleAccessTraits<
        typename tuple_element<tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value - N - 1,
            tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::type>::ConstType
    get(const tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>& t)
    {
      return Dune::get<tuple_size<tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9> >::value - N - 1>(t);
    }
  };

}

#endif
