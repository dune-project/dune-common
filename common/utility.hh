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
  template <class PairT>
  class NullPointerInitialiser {};

  /**
   * @brief Specialisation for standard tuple element.
   */
  template <class Head, class Tail>
  class NullPointerInitialiser<Pair<Head*, Tail> > {
  public:
    //! The subpart of the tuple which is handed back to the next instance
    //! of the NullPointerInitialiser.
    typedef Pair<Head*, Tail> ResultType;
  public:
    //! Static method to build up tuple.
    static inline ResultType apply() {
      return ResultType(0, NullPointerInitialiser<Tail>::apply());
    }
  };

  /**
   * @brief Specialisation for last (Nil) element.
   */
  template <>
  class NullPointerInitialiser<Nil> {
  public:
    //! The return type of the close is Nil.
    typedef Nil ResultType;
  public:
    //! Provide closure of tuple
    static inline ResultType apply() {
      return Nil();
    }
  };

  /**
   * @brief Deletes all objects pointed to in a tuple of pointers.
   *
   * \warning Pointers cannot be set to NULL, so calling the Deletor twice
   * or accessing elements of a deleted tuple leads to unforeseeable results!
   */
  template <class PairT>
  struct PointerPairDeletor {};

  /**
   * @brief Specialisation for a standard tuple element.
   */
  template <class Head, class Tail>
  struct PointerPairDeletor<Pair<Head*, Tail> > {
    //! Deletes object pointed to by first element and triggers deletion on
    //! subsequent pairs.
    static void apply(Pair<Head*, Tail>& p) {
      delete p.first();
      PointerPairDeletor<Tail>::apply(p.second());
    }
  };

  /**
   * @brief Specialisation for last (non-Nil) tuple element.
   */
  template <class Head>
  struct PointerPairDeletor<Pair<Head*, Nil> > {
    //! Deletes object pointed to by first element.
    static void apply(Pair<Head*, Nil>& p) {
      delete p.first();
    }
  };

  /**
   * @brief Helper template to calculate length of a tuple.
   */
  template <class PairT>
  struct Length {};

  /**
   * @brief Specialisation for a standard tuple element.
   *
   * The length of the tuple is stored by the enumeration value.
   */
  template <class Head, class Tail>
  struct Length<Pair<Head, Tail> > {
    //! The length of the (sub)tuple.
    enum { value = 1 + Length<Tail>::value };
  };

  /**
   * @brief Specialisation for the closure.
   */
  template <>
  struct Length<Nil> {
    //! The length of an empty tuple is zero by definition.
    enum { value = 0 };
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
  struct ForEachType {};

  /**
   * @brief Specialisation for standard tuple element
   */
  template <template <class> class TypeEvaluator, class Head, class Tail>
  struct ForEachType<TypeEvaluator, Pair<Head, Tail> > {
    //! Defines type corresponding to the subtuple defined by Pair<Head, Tail>
    typedef Pair<typename TypeEvaluator<Head>::Type,
        typename ForEachType<TypeEvaluator, Tail>::Type> Type;
  };

  /**
   * @brief Specialisation for last element
   */
  template <template <class> class TypeEvaluator>
  struct ForEachType<TypeEvaluator, Nil> {
    typedef Nil Type;
  };

  //   template <template <class> class TypeEvaluator, class Head>
  //   struct ForEachType<TypeEvaluator, Pair<Head, Nil> > {
  //     //! For the storage element, Head is replaced by the expression provided
  //     //! by the TypeEvaluator helper template.
  //     typedef Pair<typename TypeEvaluator<Head>::Type, Nil> Type;
  //   };

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
      apply(f, tuple_);
    }

  private:
    //! Specialisation for the last element
    template <class Functor, class Head>
    void apply(Functor& f, Pair<Head, Nil>& last) {
      f.visit(last.first());
    }

    //! Specialisation for a standard tuple element
    template <class Functor, class Head, class Tail>
    void apply(Functor& f, Pair<Head, Tail>& pair) {
      f.visit(pair.first());
      apply(f, pair.second());
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
      apply(f, tuple1_, tuple2_);
    }

  private:
    //! Specialisation for the last element.
    template <class Functor, class Head1, class Head2>
    void apply(Functor& f, Pair<Head1, Nil>& last1, Pair<Head2, Nil>& last2) {
      f.visit(last1.first(), last2.first());
    }

    //! Specialisation for a standard element.
    template <class Functor, class Head1, class Tail1, class Head2,class Tail2>
    void apply(Functor& f, Pair<Head1, Tail1>& p1, Pair<Head2, Tail2>& p2) {
      f.visit(p1.first(), p2.first());
      apply(f, p1.second(), p2.second());
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
    template <class T1, class T2>
    static typename ElementType<Length<Pair<T1, T2> >::value - N - 1,
        Pair<T1, T2> >::Type&
    get(Pair<T1, T2>& tuple) {
      return Element<Length<Pair<T1, T2> >::value - N - 1>::get(tuple);
    }

    template <class T1, class T2>
    static const typename ElementType<Length<Pair<T1, T2> >::value - N - 1,
        Pair<T1, T2> >::Type&
    get(const Pair<T1, T2>& tuple) {
      return Element<Length<Pair<T1, T2> >::value - N - 1>::get(tuple);
    }

  };

}

#endif
