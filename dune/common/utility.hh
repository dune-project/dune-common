// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UTILITY_HH
#define DUNE_UTILITY_HH

#include <dune/common/static_assert.hh>

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
  class NullPointerInitialiser {
    dune_static_assert(AlwaysFalse<Tuple>::value, "Attempt to use the "
                       "unspecialized version of NullPointerInitialiser.  "
                       "NullPointerInitialiser needs to be specialized for "
                       "each possible tuple size.  Naturally the number of "
                       "pre-defined specializations is limited arbitrarily.  "
                       "Maybe you need to raise this limit by defining some "
                       "more specializations?  Also check that the tuple this "
                       "is applied to really is a tuple of pointers only.");
  public:
    //! export the type of the tuples
    typedef Tuple ResultType;
    //! generate a zero-initialized tuple
    static ResultType apply();
  };

#ifndef DOXYGEN
  template<class Tuple>
  struct NullPointerInitialiser<const Tuple>
    : public NullPointerInitialiser<Tuple>
  {
    typedef const Tuple ResultType;
  };

  template<>
  struct NullPointerInitialiser<tuple<> > {
    typedef tuple<> ResultType;
    static ResultType apply() {
      return ResultType();
    }
  };

  template<class T0>
  struct NullPointerInitialiser<tuple<T0*> > {
    typedef tuple<T0*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0));
    }
  };

  template<class T0, class T1>
  struct NullPointerInitialiser<tuple<T0*, T1*> > {
    typedef tuple<T0*, T1*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0));
    }
  };

  template<class T0, class T1, class T2>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*> > {
    typedef tuple<T0*, T1*, T2*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*> > {
    typedef tuple<T0*, T1*, T2*, T3*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4, class T5>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0), static_cast<T5*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0), static_cast<T5*>(0),
                        static_cast<T6*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*,
          T7*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0), static_cast<T5*>(0),
                        static_cast<T6*>(0), static_cast<T7*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class T8>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*,
          T7*, T8*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*, T8*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0), static_cast<T5*>(0),
                        static_cast<T6*>(0), static_cast<T7*>(0),
                        static_cast<T8*>(0));
    }
  };

  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class T8, class T9>
  struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*,
          T7*, T8*, T9*> > {
    typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*, T8*, T9*> ResultType;
    static ResultType apply() {
      return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
                        static_cast<T2*>(0), static_cast<T3*>(0),
                        static_cast<T4*>(0), static_cast<T5*>(0),
                        static_cast<T6*>(0), static_cast<T7*>(0),
                        static_cast<T8*>(0), static_cast<T9*>(0));
    }
  };
#endif // !defined(DOXYGEN)

  /**
   * @brief Helper template to calculate length of a tuple.
   */
  template <class Tuple>
  struct Length
  {
    enum {
      value=tuple_size<Tuple>::value
    };
  };

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
  class ForEachType {
    dune_static_assert(AlwaysTrue<TupleType>::value, "Attempt to use the "
                       "unspecialized version of ForEachType.  ForEachType "
                       "needs to be specialized for each possible tuple "
                       "size.  Naturally the number of pre-defined "
                       "specializations is limited arbitrarily.  Maybe you "
                       "need to raise this limit by defining some more "
                       "specializations?");
    struct ImplementationDefined {};
  public:
    //! type of the transformed tuple
    typedef ImplementationDefined Type;
  };

#ifndef DOXYGEN
  template <template <class> class TE>
  struct ForEachType<TE, tuple<> > {
    typedef tuple<> Type;
  };

  template <template <class> class TE, class T0>
  struct ForEachType<TE, tuple<T0> > {
    typedef tuple<typename TE<T0>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1>
  struct ForEachType<TE, tuple<T0, T1> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2>
  struct ForEachType<TE, tuple<T0, T1, T2> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3>
  struct ForEachType<TE, tuple<T0, T1, T2, T3> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4, class T5>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type, typename TE<T5>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4, class T5, class T6>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5, T6> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type, typename TE<T5>::Type,
        typename TE<T6>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4, class T5, class T6, class T7>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5, T6, T7> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type, typename TE<T5>::Type,
        typename TE<T6>::Type, typename TE<T7>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4, class T5, class T6, class T7, class T8>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type, typename TE<T5>::Type,
        typename TE<T6>::Type, typename TE<T7>::Type,
        typename TE<T8>::Type> Type;
  };

  template <template <class> class TE, class T0, class T1, class T2, class T3,
      class T4, class T5, class T6, class T7, class T8, class T9>
  struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> > {
    typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
        typename TE<T2>::Type, typename TE<T3>::Type,
        typename TE<T4>::Type, typename TE<T5>::Type,
        typename TE<T6>::Type, typename TE<T7>::Type,
        typename TE<T8>::Type, typename TE<T9>::Type> Type;
  };
#endif // !defined(DOXYGEN)

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
    typedef typename tuple_element<Length<Tuple>::value - N - 1,
        Tuple>::type Type;
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

    template<typename Tuple>
    static
    typename TupleAccessTraits<typename AtType<N, Tuple>::Type>::NonConstType
    get(Tuple& t)
    {
      return Dune::get<tuple_size<Tuple>::value - N - 1>(t);
    }

    template<typename Tuple>
    static
    typename TupleAccessTraits<typename AtType<N, Tuple>::Type>::ConstType
    get(const Tuple& t)
    {
      return Dune::get<tuple_size<Tuple>::value - N - 1>(t);
    }
  };

  /**
   * @brief Deletes all objects pointed to in a tuple of pointers.
   *
   * \warning Pointers cannot be set to NULL, so calling the Deletor twice
   * or accessing elements of a deleted tuple leads to unforeseeable results!
   */
  template <class Tuple>
  class PointerPairDeletor
  {
    struct Deletor {
      template<typename P> void visit(const P& p) { delete p; }
    };

  public:
    static void apply(Tuple& t) {
      static Deletor deletor;
      ForEachValue<Tuple>(t).apply(deletor);
    }
  };

}

#endif
