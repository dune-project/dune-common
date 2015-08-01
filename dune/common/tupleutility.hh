// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TUPLE_UTILITY_HH
#define DUNE_TUPLE_UTILITY_HH

#include <cstddef>

#include <dune/common/typetraits.hh>
#include <dune/common/std/type_traits.hh>

#include "tuples.hh"

namespace Dune {

  /** @ addtogroup Common
   *
   * @{
   */

  /**
   * @file
   * @brief Contains utility classes which can be used with tuples.
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
    static_assert(Std::to_false_type<Tuple>::value, "Attempt to use the "
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

  // template<class T0, class T1, class T2, class T3, class T4, class T5,
  //          class T6, class T7, class T8, class T9>
  // struct NullPointerInitialiser<tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*,
  //                                     T7*, T8*, T9*> > {
  //   typedef tuple<T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*, T8*, T9*> ResultType;
  //   static ResultType apply() {
  //     return ResultType(static_cast<T0*>(0), static_cast<T1*>(0),
  //                       static_cast<T2*>(0), static_cast<T3*>(0),
  //                       static_cast<T4*>(0), static_cast<T5*>(0),
  //                       static_cast<T6*>(0), static_cast<T7*>(0),
  //                       static_cast<T8*>(0), static_cast<T9*>(0));
  //   }
  // };
#endif // !defined(DOXYGEN)

  /**
   * @brief Helper template to clone the type definition of a tuple with the
   * storage types replaced by a user-defined rule.
   *
   * Suppose all storage types A_i in a tuple define a type A_i::B. You can
   * build up a pair consisting of the types defined by A_i::B in the following
   * way:
   * \code
   * template <class A>
   * struct MyEvaluator {
   *   typedef typename A::B Type;
   * };
   *
   * typedef ForEachType<MyEvaluator, ATuple>::Type BTuple;
   * \endcode
   * Here, MyEvaluator is a helper struct that extracts the correct type from
   * the storage types of the tuple defined by the tuple ATuple.
   *
   * \sa AddRefTypeEvaluator, AddPtrTypeEvaluator, genericTransformTuple(),
   *     and transformTuple().
   */
  template <template <class> class TypeEvaluator, class TupleType>
  class ForEachType {
    static_assert(Std::to_false_type<TupleType>::value, "Attempt to use the "
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
  template <template <class> class TE, class Tuple>
  struct ForEachType<TE, const Tuple> {
    typedef const typename ForEachType<TE, Tuple>::Type Type;
  };

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

  // template <template <class> class TE, class T0, class T1, class T2, class T3,
  //           class T4, class T5, class T6, class T7, class T8, class T9>
  // struct ForEachType<TE, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> > {
  //   typedef tuple<typename TE<T0>::Type, typename TE<T1>::Type,
  //                 typename TE<T2>::Type, typename TE<T3>::Type,
  //                 typename TE<T4>::Type, typename TE<T5>::Type,
  //                 typename TE<T6>::Type, typename TE<T7>::Type,
  //                 typename TE<T8>::Type, typename TE<T9>::Type> Type;
  // };
#endif // !defined(DOXYGEN)

  //////////////////////////////////////////////////////////////////////
  //
  // genericTransformTuple stuff
  //

  // genericTransformTuple() needs to be overloaded for each tuple size (we
  // limit ourselves to tuple_size <= 10 here).  For a given tuple size it
  // needs to be overloaded for all combinations of const and non-const
  // argument references.  (On the one hand, we want to allow modifyable
  // arguments, so const references alone are not sufficient.  On the other
  // hand, we also want to allow rvalues (literals) as argument, which do not
  // bind to non-const references.)
  //
  // We can half the number of specializations required by introducing a
  // function genericTransformTupleBackend(), which is overloaded for each
  // tuple size and for const and non-const tuple arguments; the functor
  // argument is always given as as (non-const) reference.  When
  // genericTransformTupleBackend() is called, the type of the Functor template
  // parameter is the deduced as either "SomeType" or "const SomeType",
  // depending on whether the function argument is a non-const or a const
  // lvalue of type "SomeType".  As explained above, this does not work for
  // rvalues (i.e. literals).
  //
  // To make it work for literals of functors as well, we wrap the call to
  // genericTransformTupleBackend() in a function genericTransformTuple().
  // genericTransformTuple() needs to be overloaded for non-const and const
  // tuples and functors -- 4 overloads only.  Inside genericTransformTuple()
  // the functor is an lvalue no matter whether the argument was an lvalue or
  // an rvalue.  There is no need need to overload genericTransformTuple() for
  // all tuple sizes -- this is done by the underlying
  // genericTransformTupleBackend().

  // genericTransformTupleBackend() is an implementation detail -- hide it
  // from Doxygen
#ifndef DOXYGEN
  // 0-element tuple
  // This is a special case: we touch neither the tuple nor the functor, so
  // const references are sufficient and we don't need to overload
  template<class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<> >::Type
  genericTransformTupleBackend
    (const tuple<>& t, const Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<> >::Type
             ();
  }

  // 1-element tuple
  template<class T0, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0> >::Type
  genericTransformTupleBackend
    (tuple<T0>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0> >::Type
             (f(get<0>(t)));
  }
  template<class T0, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0> >::Type
  genericTransformTupleBackend
    (const tuple<T0>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0> >::Type
             (f(get<0>(t)));
  }

  // 2-element tuple
  template<class T0, class T1, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1> >::Type
             (f(get<0>(t)), f(get<1>(t)));
  }
  template<class T0, class T1, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1> >::Type
             (f(get<0>(t)), f(get<1>(t)));
  }

  // 3-element tuple
  template<class T0, class T1, class T2, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)));
  }
  template<class T0, class T1, class T2, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)));
  }

  // 4-element tuple
  template<class T0, class T1, class T2, class T3, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)));
  }
  template<class T0, class T1, class T2, class T3, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)));
  }

  // 5-element tuple
  template<class T0, class T1, class T2, class T3, class T4, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3, T4>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)));
  }
  template<class T0, class T1, class T2, class T3, class T4, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3, T4>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)));
  }

  // 6-element tuple
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3, T4, T5>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)));
  }
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3, T4, T5>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)));
  }

  // 7-element tuple
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3, T4, T5, T6>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)));
  }
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3, T4, T5, T6>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)));
  }

  // 8-element tuple
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6, T7> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3, T4, T5, T6, T7>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6, T7> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)), f(get<7>(t)));
  }
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6, T7> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3, T4, T5, T6, T7>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6, T7> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)), f(get<7>(t)));
  }

  // 9-element tuple
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class T8, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> >::Type
  genericTransformTupleBackend
    (tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)), f(get<7>(t)), f(get<8>(t)));
  }
  template<class T0, class T1, class T2, class T3, class T4, class T5,
      class T6, class T7, class T8, class Functor>
  typename ForEachType<Functor::template TypeEvaluator,
      tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> >::Type
  genericTransformTupleBackend
    (const tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>& t, Functor& f)
  {
    return typename ForEachType<Functor::template TypeEvaluator,
        tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> >::Type
             (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
             f(get<5>(t)), f(get<6>(t)), f(get<7>(t)), f(get<8>(t)));
  }

  // // 10-element tuple
  // template<class T0, class T1, class T2, class T3, class T4, class T5,
  //          class T6, class T7, class T8, class T9, class Functor>
  // typename ForEachType<Functor::template TypeEvaluator,
  //                      tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::Type
  // genericTransformTupleBackend
  // (tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& t, Functor& f)
  // {
  //   return typename ForEachType<Functor::template TypeEvaluator,
  //     tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::Type
  //     (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
  //      f(get<5>(t)), f(get<6>(t)), f(get<7>(t)), f(get<8>(t)), f(get<9>(t)));
  // }
  // template<class T0, class T1, class T2, class T3, class T4, class T5,
  //          class T6, class T7, class T8, class T9, class Functor>
  // typename ForEachType<Functor::template TypeEvaluator,
  //                      tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::Type
  // genericTransformTupleBackend
  // (const tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>& t, Functor& f)
  // {
  //   return typename ForEachType<Functor::template TypeEvaluator,
  //     tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::Type
  //     (f(get<0>(t)), f(get<1>(t)), f(get<2>(t)), f(get<3>(t)), f(get<4>(t)),
  //      f(get<5>(t)), f(get<6>(t)), f(get<7>(t)), f(get<8>(t)), f(get<9>(t)));
  // }
#endif // ! defined(DOXYGEN)

  //! transform a tuple object into another tuple object
  /**
   * \code
   * #include <dune/common/tupleutility.hh>
   * \endcode
   * This function does for the value of a tuple what ForEachType does for the
   * type of a tuple: it transforms the value using a user-provided policy
   * functor.
   *
   * \param t The tuple value to transform.
   * \param f The functor to use to transform the values.
   *
   * The functor should have the following form:
   * \code
   * struct Functor {
   *   template<class> struct TypeEvaluator {
   *     typedef user-defined Type;
   *   };
   *
   *   template<class T>
   *   typename TypeEvaluator<T>::Type operator()(T& val);
   *
   *   template<class T>
   *   typename TypeEvaluator<T>::Type operator()(T& val) const;
   *
   *   template<class T>
   *   typename TypeEvaluator<T>::Type operator()(const T& val);
   *
   *   template<class T>
   *   typename TypeEvaluator<T>::Type operator()(const T& val) const;
   * };
   * \endcode
   * The member class template \c TypeEvaluator should be a class template
   * suitable as the \c TypeEvaluator template parameter for ForEachType.  The
   * function call operator \c operator() is used to transform the value; only
   * the signatures of \c operator() which are actually used must be present.
   *
   * There are overloaded definitions of genericTransformTuple() wich take
   * constant tuple and functor arguments so rvalues are permissible as
   * arguments here.  These overloaded definitions are not documented
   * separately.
   */
  template<class Tuple, class Functor>
  typename ForEachType<Functor::template TypeEvaluator, Tuple>::Type
  genericTransformTuple(Tuple& t, Functor& f) {
    return genericTransformTupleBackend(t, f);
  }
#ifndef DOXYGEN
  template<class Tuple, class Functor>
  typename ForEachType<Functor::template TypeEvaluator, Tuple>::Type
  genericTransformTuple(const Tuple& t, Functor& f) {
    return genericTransformTupleBackend(t, f);
  }
  template<class Tuple, class Functor>
  typename ForEachType<Functor::template TypeEvaluator, Tuple>::Type
  genericTransformTuple(Tuple& t, const Functor& f) {
    return genericTransformTupleBackend(t, f);
  }
  template<class Tuple, class Functor>
  typename ForEachType<Functor::template TypeEvaluator, Tuple>::Type
  genericTransformTuple(const Tuple& t, const Functor& f) {
    return genericTransformTupleBackend(t, f);
  }
#endif // ! defined(DOXYGEN)

  ////////////////////////////////////////////////////////////////////////
  //
  // transformTuple() related stuff
  //

  //! helper class to implement transformTuple()
  /**
   * \tparam TE TypeEvaluator class template.
   * \tparam An Type of extra arguments to pass to \c TE<T>::apply().  \c void
   *            means "no argument".  Only trailing arguments may be void.
   *
   * This class stores references to a number of arguments it receives in the
   * constructor.  Later, its function call operator \c operator() may be
   * called with a parameter \c t of type \c T.  \c operator() will then call
   * the static method \c TE<T>::apply(t,args...), where \c args... is the
   * sequence of arguments the object was constructed with.  \c operator()
   * will convert the result to type \c TE<T>::Type and return it.
   *
   * \c TE should be an extended version of the \c TypeEvaluator class
   * template parameter of ForEachType, for instance:
   * \code
   * template <class T>
   * struct TypeEvaluator {
   *   typedef T* Type;
   *   static Type apply(T& t, void* a0) {
   *     return t ? &t : static_cast<T*>(a0);
   *   }
   * };
   * \endcode
   * This example is for a TransformTupleFunctor with one argument, i.e. \c
   * A0!=void and all other \c An=void.  For the type transformation, it will
   * transform a value of some type T into a pointer to T.  For the value
   * transformation, it will take a reference to a value of type T and return
   * the pointer to that value, unless the value evaluates to false in boolean
   * context.  If the value evaluates to false, it will instead return the
   * pointer from the extra argument.
   */
  template<template<class> class TE, class A0 = void, class A1 = void,
      class A2 = void, class A3 = void, class A4 = void, class A5 = void,
      class A6 = void, class A7 = void, class A8 = void, class A9 = void>
  class TransformTupleFunctor {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4; A5& a5; A6& a6; A7& a7; A8& a8;
    A9& a9;

  public:
    //! export the TypeEvaluator template class for genericTransformTuple()
    template<class T> struct TypeEvaluator : public TE<T> {};

    //! constructor
    /**
     * The actual number of arguments varies between specializations, the
     * actual number of arguments here is equal to the number of non-\c void
     * class template arguments \c An.
     */
    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_, A5& a5_,
                          A6& a6_, A7& a7_, A8& a8_, A9& a9_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_), a5(a5_), a6(a6_), a7(a7_),
        a8(a8_), a9(a9_)
    { }

    //! call \c TE<T>::apply(t,args...)
    /**
     * This calls the static apply method of the TypeEvaluator class
     * template.
     *
     * \note There is no need to overload \c operator() with at \c const \c T&
     * argument, since genericTransformTuple() will always use an lvalue
     * argument.
     */
    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
  };

  //! syntactic sugar for creation of TransformTupleFunctor objects
  /**
   * \code
   * #include <dune/common/tupleutility.hh>
   * \endcode
   * \tparam TE TypeEvaluator class template.
   * \tparam A0 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A1 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A2 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A3 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A4 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A5 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A6 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A7 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A8 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   * \tparam A9 Type of extra arguments to pass to \c TE<T>::apply().  It
   *            should not be necessary to specify these template parameters
   *            explicitly since they can be deduced.
   *
   * \param a0 Arguments to save references to in the TransformTupleFunctor.
   * \param a1 Arguments to save references to in the TransformTupleFunctor.
   * \param a2 Arguments to save references to in the TransformTupleFunctor.
   * \param a3 Arguments to save references to in the TransformTupleFunctor.
   * \param a4 Arguments to save references to in the TransformTupleFunctor.
   * \param a5 Arguments to save references to in the TransformTupleFunctor.
   * \param a6 Arguments to save references to in the TransformTupleFunctor.
   * \param a7 Arguments to save references to in the TransformTupleFunctor.
   * \param a8 Arguments to save references to in the TransformTupleFunctor.
   * \param a9 Arguments to save references to in the TransformTupleFunctor.
   *
   * There are overloads of this function (not documented separately) for any
   * number of arguments, up to an implementation-defined arbitrary limit.
   * The number of arguments given determines the number of non-\c void
   * template arguments in the type of the returned TransformTupleFunctor.
   */
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6, class A7, class A8, class A9>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                            A6& a6, A7& a7, A8& a8, A9& a9) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>
             (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

#ifndef DOXYGEN
  // 0 argument
  template<template<class> class TE>
  struct TransformTupleFunctor<TE>
  {
    template<class T> struct TypeEvaluator : public TE<T> {};

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t);
    }
  };
  template<template<class> class TE>
  TransformTupleFunctor<TE>
  makeTransformTupleFunctor() {
    return TransformTupleFunctor<TE>
             ();
  }

  // 1 argument
  template<template<class> class TE, class A0>
  class TransformTupleFunctor<TE, A0>
  {
    A0& a0;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_)
      : a0(a0_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0);
    }
  };
  template<template<class> class TE, class A0>
  TransformTupleFunctor<TE, A0>
  makeTransformTupleFunctor(A0& a0) {
    return TransformTupleFunctor<TE, A0>
             (a0);
  }

  // 2 argument
  template<template<class> class TE, class A0, class A1>
  class TransformTupleFunctor<TE, A0, A1>
  {
    A0& a0; A1& a1;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_)
      : a0(a0_), a1(a1_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1);
    }
  };
  template<template<class> class TE, class A0, class A1>
  TransformTupleFunctor<TE, A0, A1>
  makeTransformTupleFunctor(A0& a0, A1& a1) {
    return TransformTupleFunctor<TE, A0, A1>
             (a0, a1);
  }

  // 3 arguments
  template<template<class> class TE, class A0, class A1, class A2>
  class TransformTupleFunctor<TE, A0, A1, A2>
  {
    A0& a0; A1& a1; A2& a2;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_)
      : a0(a0_), a1(a1_), a2(a2_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2>
  TransformTupleFunctor<TE, A0, A1, A2>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2) {
    return TransformTupleFunctor<TE, A0, A1, A2>
             (a0, a1, a2);
  }

  // 4 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3>
  class TransformTupleFunctor<TE, A0, A1, A2, A3>
  {
    A0& a0; A1& a1; A2& a2; A3& a3;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3>
  TransformTupleFunctor<TE, A0, A1, A2, A3>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3>
             (a0, a1, a2, a3);
  }

  // 5 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4>
  class TransformTupleFunctor<TE, A0, A1, A2, A3, A4>
  {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4>
             (a0, a1, a2, a3, a4);
  }

  // 6 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5>
  class TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5>
  {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4; A5& a5;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_, A5& a5_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_), a5(a5_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4, a5);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5>
             (a0, a1, a2, a3, a4, a5);
  }

  // 7 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6>
  class TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6>
  {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4; A5& a5; A6& a6;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_, A5& a5_,
                          A6& a6_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_), a5(a5_), a6(a6_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4, a5, a6);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                            A6& a6) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6>
             (a0, a1, a2, a3, a4, a5, a6);
  }

  // 8 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6, class A7>
  class TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7>
  {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4; A5& a5; A6& a6; A7& a7;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_, A5& a5_,
                          A6& a6_, A7& a7_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_), a5(a5_), a6(a6_), a7(a7_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4, a5, a6, a7);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6, class A7>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                            A6& a6, A7& a7) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7>
             (a0, a1, a2, a3, a4, a5, a6, a7);
  }

  // 9 arguments
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6, class A7, class A8>
  class TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7, A8>
  {
    A0& a0; A1& a1; A2& a2; A3& a3; A4& a4; A5& a5; A6& a6; A7& a7; A8& a8;

  public:
    template<class T> struct TypeEvaluator : public TE<T> {};

    TransformTupleFunctor(A0& a0_, A1& a1_, A2& a2_, A3& a3_, A4& a4_, A5& a5_,
                          A6& a6_, A7& a7_, A8& a8_)
      : a0(a0_), a1(a1_), a2(a2_), a3(a3_), a4(a4_), a5(a5_), a6(a6_), a7(a7_),
        a8(a8_)
    { }

    template<class T>
    typename TE<T>::Type operator()(T& t) const {
      return TE<T>::apply(t, a0, a1, a2, a3, a4, a5, a6, a7, a8);
    }
  };
  template<template<class> class TE, class A0, class A1, class A2, class A3,
      class A4, class A5, class A6, class A7, class A8>
  TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7, A8>
  makeTransformTupleFunctor(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                            A6& a6, A7& a7, A8& a8) {
    return TransformTupleFunctor<TE, A0, A1, A2, A3, A4, A5, A6, A7, A8>
             (a0, a1, a2, a3, a4, a5, a6, a7, a8);
  }
#endif // ! defined(DOXYGEN)

  //! transform a tuple's value according to a user-supplied policy
  /**
   * \code
   * #include <dune/common/tupleutility.hh>
   * \endcode
   * This function provides functionality similar to genericTransformTuple(),
   * although less general and closer in spirit to ForEachType.
   *
   * \tparam TypeEvaluator Used as the \c TE template argument to
   *                       TransformTupleFunctor internally.
   * \tparam Tuple         Type of the tuple to transform.
   * \tparam A0            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A1            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A2            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A3            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A4            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A5            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A6            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A7            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A8            Types of extra argument to call the transformation
   *                       function with.
   * \tparam A9            Types of extra argument to call the transformation
   *                       function with.
   *
   * \note The \c Tuple and \c An template arguments can be deduced from the
   *       function arguments, so they can usually be omitted.
   *
   * \param orig Tuple value to be transformed.
   * \param a0   Extra argument values to provide to the transformation
   *             function.
   * \param a1   Extra argument values to provide to the transformation
   *             function.
   * \param a2   Extra argument values to provide to the transformation
   *             function.
   * \param a3   Extra argument values to provide to the transformation
   *             function.
   * \param a4   Extra argument values to provide to the transformation
   *             function.
   * \param a5   Extra argument values to provide to the transformation
   *             function.
   * \param a6   Extra argument values to provide to the transformation
   *             function.
   * \param a7   Extra argument values to provide to the transformation
   *             function.
   * \param a8   Extra argument values to provide to the transformation
   *             function.
   * \param a9   Extra argument values to provide to the transformation
   *             function.
   *
   * This function is overloaded for any number of extra arguments, up to an
   * implementation-defined arbitrary limit.  The overloads are not documented
   * separately.
   *
   * The \c TypeEvaluator class template should be suitable as the \c TE
   * template argument for TransformTupleFunctor.  It has the following form
   * (an extension of the \c TypeEvaluator template argument of ForEachType):
   * \code
   * template <class T>
   * struct TypeEvaluator {
   *   typedef UserDefined Type;
   *
   *   template<class A0, class A1, class A2, class A3, class A4, class A5,
   *            class A6, class A7, class A8, class A9>
   *   static Type apply(T& t, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
   *                     A6& a6, A7& a7, A8& a8, A9& a9);
   * };
   * \endcode
   * For any given element type \c T of the tuple, the TypeEvaluator template
   * class should provide a member typedef \c Type which determines the type
   * of the transformed value and a static function \c apply(), taking the
   * value of the tuple element \c t and the extra arguments given to
   * transformTuple().  The signature of \c apply() does not have to match the
   * one given above exactly, as long as it can be called that way.
   *
   * \note Since transformTuple() takes non-const references to the extra
   *       arguments, it will only bind to lvalue extra arguments, unless you
   *       specify the corresponding template parameter as \c const \c
   *       SomeType.  Specifically this means that you cannot simply use
   *       literals or function return values as extra arguments. Providing
   *       overloads for all possible combinations of rvalue and lvalue extra
   *       arguments would result in \f$2^{n+1}-1\f$ overloads where \f$n\f$
   *       is the implementation defined limit in the number of extra
   *       arguments.
   *
   * \sa genericTransforTuple(), ForEachType, AddRefTypeEvaluator, and
   *     AddPtrTypeEvaluator.
   */
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4, class A5, class A6,
      class A7, class A8, class A9>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                 A6& a6, A7& a7, A8& a8, A9& a9) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4, a5, a6,
                                                      a7, a8, a9));
  }

#ifndef DOXYGEN
  // 0 extra arguments
  template<template<class> class TypeEvaluator, class Tuple>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>());
  }

  // 1 extra argument
  template<template<class> class TypeEvaluator, class Tuple, class A0>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0));
  }

  // 2 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1));
  }

  // 3 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2));
  }

  // 4 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3));
  }

  // 5 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4));
  }

  // 6 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4, class A5>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4, a5));
  }

  // 7 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4, class A5, class A6>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                 A6& a6) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4, a5, a6));
  }

  // 8 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4, class A5, class A6,
      class A7>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                 A6& a6, A7& a7) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4, a5, a6,
                                                      a7));
  }

  // 9 extra arguments
  template<template<class> class TypeEvaluator, class Tuple, class A0,
      class A1, class A2, class A3, class A4, class A5, class A6,
      class A7, class A8>
  typename remove_const<typename ForEachType<TypeEvaluator, Tuple>::Type>::type
  transformTuple(Tuple& orig, A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5,
                 A6& a6, A7& a7, A8& a8) {
    return genericTransformTuple
             ( orig,
             makeTransformTupleFunctor<TypeEvaluator>(a0, a1, a2, a3, a4, a5, a6,
                                                      a7, a8));
  }
#endif // not defined(DOXYGEN)

  ////////////////////////////////////////////////////////////////////////
  //
  // Sample TypeEvaluators
  //

  //! \c TypeEvaluator to turn a type \c T into a reference to \c T
  /**
   * This is suitable as the \c TypeEvaluator template parameter for
   * ForEachType and transformTuple().
   */
  template<class T>
  struct AddRefTypeEvaluator {
    typedef T& Type;
    static Type apply(T& t) { return t; }
  };

  //! \c TypeEvaluator to turn a type \c T into a pointer to \c T
  /**
   * This is suitable as the \c TypeEvaluator template parameter for
   * ForEachType and transformTuple().
   */
  template<class T>
  struct AddPtrTypeEvaluator {
    typedef typename remove_reference<T>::type* Type;
    static Type apply(T& t) { return &t; }
  };

  // Specialization, in case the type is already a reference
  template<class T>
  struct AddPtrTypeEvaluator<T&> {
    typedef typename remove_reference<T>::type* Type;
    static Type apply(T& t) { return &t; }
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
      static inline void visit(F&, T1&)
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
      static inline void visit(F&, T1&, T2&)
      {}
    };
  }

  /**
   * @brief Helper template which implements iteration over all storage
   * elements in a tuple.
   *
   * Compile-time constructs that allows one to process all elements in a tuple.
   * The exact operation performed on an element is defined by a function
   * object, which needs to implement a visit method which is applicable to
   * all storage elements of a tuple.  Each tuple element is visited once, and
   * the iteration is done in ascending order.
   *
   * The following example implements a function object which counts the
   * elements in a tuple
   * \code
   * template <class T>
   * struct Counter {
   * Counter() : result_(0) {}
   *
   * template <class T>
   * void visit(T& elem) { ++result_; }
   *
   * int result_;
   * };
   * \endcode
   * The number of elements in the tuple are stored in the member variable
   * result_. The Counter can be used as follows, assuming a tuple t of type
   * MyTuple is given:
   * \code
   * Counter c;
   * ForEachValue<MyTuple> forEach(t);
   *
   * forEach.apply(c);
   * std::cout << "Number of elements is: " << c.result_ << std::endl;
   * \endcode
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
    void apply(Functor& f) const {
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
    typedef typename tuple_element<tuple_size<Tuple>::value - N - 1,
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


  /**
   * @brief Finding the index of a certain type in a tuple
   *
   * \tparam Tuple     The tuple type to search in.
   * \tparam Predicate Predicate which tells FirstPredicateIndex which types
   *                   in Tuple to accept.  This should be a class template
   *                   taking a single type template argument.  When
   *                   instantiated, it should contain a static member
   *                   constant \c value which should be convertible to bool.
   *                   A type is accepted if \c value is \c true, otherwise it
   *                   is rejected and the next type is tried.  Look at IsType
   *                   for a sample implementation.
   * \tparam start     First index to try.  This can be adjusted to skip
   *                   leading tuple elements.
   * \tparam size      This parameter is an implementation detail and should
   *                   not be adjusted by the users of this class.  It should
   *                   always be equal to the size of the tuple.
   *
   * This class can search for a type in tuple.  It will apply the predicate
   * to each type in tuple in turn, and set its member constant \c value to
   * the index of the first type that was accepted by the predicate.  If none
   * of the types are accepted by the predicate, a static_assert is triggered.
   */
  template<class Tuple, template<class> class Predicate, std::size_t start = 0,
      std::size_t size = tuple_size<Tuple>::value>
  class FirstPredicateIndex :
    public conditional<Predicate<typename tuple_element<start,
                Tuple>::type>::value,
        integral_constant<std::size_t, start>,
        FirstPredicateIndex<Tuple, Predicate, start+1> >::type
  {
    static_assert(tuple_size<Tuple>::value == size, "The \"size\" "
                       "template parameter of FirstPredicateIndex is an "
                       "implementation detail and should never be set "
                       "explicitly!");
  };

#ifndef DOXYGEN
  template<class Tuple, template<class> class Predicate, std::size_t size>
  class FirstPredicateIndex<Tuple, Predicate, size, size>
  {
    static_assert(Std::to_false_type<Tuple>::value, "None of the tuple element "
                       "types matches the predicate!");
  };
#endif // !DOXYGEN

  /**
   * @brief Generator for predicates accepting one particular type
   *
   * \tparam T The type to accept.
   *
   * The generated predicate class is useful together with
   * FirstPredicateIndex.  It will accept exactly the type that is given as
   * the \c T template parameter.
   */
  template<class T>
  struct IsType {
    //! @brief The actual predicate
    template<class U>
    struct Predicate : public is_same<T, U> {};
  };

  /**
   * @brief Find the first occurance of a type in a tuple
   *
   * \tparam Tuple The tuple type to search in.
   * \tparam T     Type to search for.
   * \tparam start First index to try.  This can be adjusted to skip leading
   *               tuple elements.
   *
   * This class can search for a particular type in tuple.  It will check each
   * type in the tuple in turn, and set its member constant \c value to the
   * index of the first occurance of type was found.  If the type was not
   * found, a static_assert is triggered.
   */
  template<class Tuple, class T, std::size_t start = 0>
  struct FirstTypeIndex :
    public FirstPredicateIndex<Tuple, IsType<T>::template Predicate, start>
  { };



  /**
   * \brief Helper template to append a type to a tuple
   *
   * \tparam Tuple The tuple type to extend
   * \tparam T     The type to be appended to the tuple
   */
  template< class Tuple, class T>
  struct PushBackTuple
  {
    static_assert(Std::to_false_type<Tuple>::value, "Attempt to use the "
                       "unspecialized version of PushBackTuple.  "
                       "PushBackTuple needs to be specialized for "
                       "each possible tuple size.  Naturally the number of "
                       "pre-defined specializations is limited arbitrarily.  "
                       "Maybe you need to raise this limit by defining some "
                       "more specializations?");

    /**
     * \brief For all specializations this is the type of a tuple with T appended.
     *
     * Suppose you have Tuple=tuple<T1, T2, ..., TN> then
     * this type is tuple<T1, T2, ..., TN, T>.
     */
    typedef Tuple type;
  };


#ifndef DOXYGEN

  template<class... TupleArgs, class T>
  struct PushBackTuple<typename Dune::tuple<TupleArgs...>, T>
  {
    typedef typename Dune::tuple<TupleArgs..., T> type;
  };

#endif



  /**
   * \brief Helper template to prepend a type to a tuple
   *
   * \tparam Tuple The tuple type to extend
   * \tparam T     The type to be prepended to the tuple
   */
  template< class Tuple, class T>
  struct PushFrontTuple
  {
    static_assert(Std::to_false_type<Tuple>::value, "Attempt to use the "
                       "unspecialized version of PushFrontTuple.  "
                       "PushFrontTuple needs to be specialized for "
                       "each possible tuple size.  Naturally the number of "
                       "pre-defined specializations is limited arbitrarily.  "
                       "Maybe you need to raise this limit by defining some "
                       "more specializations?");

    /**
     * \brief For all specializations this is the type of a tuple with T prepended.
     *
     * Suppose you have Tuple=tuple<T1, T2, ..., TN> then
     * this type is tuple<T, T1, T2, ..., TN>.
     */
    typedef Tuple type;
  };


#ifndef DOXYGEN

  template<class... TupleArgs, class T>
  struct PushFrontTuple<typename Dune::tuple<TupleArgs...>, T>
  {
    typedef typename Dune::tuple<T, TupleArgs...> type;
  };

#endif



  /**
   * \brief Apply reduce with meta binary function to template
   *
   * For a tuple\<T0,T1,...,TN-1,TN,...\> the exported result is
   *
   * F\< ... F\< F\< F\<Seed,T0\>\::type, T1\>\::type, T2\>\::type,  ... TN-1\>\::type
   *
   * \tparam F Binary meta function
   * \tparam Tuple Apply reduce operation to this tuple
   * \tparam Seed Initial value for reduce operation
   * \tparam N Reduce the first N tuple elements
   */
  template<
      template <class, class> class F,
      class Tuple,
      class Seed=tuple<>,
      int N=tuple_size<Tuple>::value>
  struct ReduceTuple
  {
    typedef typename ReduceTuple<F, Tuple, Seed, N-1>::type Accumulated;
    typedef typename tuple_element<N-1, Tuple>::type Value;

    //! Result of the reduce operation
    typedef typename F<Accumulated, Value>::type type;
  };

  /**
   * \brief Apply reduce with meta binary function to template
   *
   * Specialization for reduction of 0 elements.
   * The exported result type is Seed.
   *
   * \tparam F Binary meta function
   * \tparam Tuple Apply reduce operation to this tuple
   * \tparam Seed Initial value for reduce operation
   */
  template<
      template <class, class> class F,
      class Tuple,
      class Seed>
  struct ReduceTuple<F, Tuple, Seed, 0>
  {
    //! Result of the reduce operation
    typedef Seed type;
  };



  /**
   * \brief Join two tuples
   *
   * For Head=tuple<T0,...,TN> and Tail=tuple<S0,...,SM>
   * the exported result is tuple<T0,..,TN,S0,...,SM>.
   *
   * \tparam Head Head of resulting tuple
   * \tparam Tail Tail of resulting tuple
   */
  template<class Head, class Tail>
  struct JoinTuples
  {
    //! Result of the join operation
    typedef typename ReduceTuple< PushBackTuple, Tail, Head>::type type;
  };



  /**
   * \brief Flatten a tuple of tuples
   *
   * This flattens a tuple of tuples tuple<tuple<T0,...,TN>, tuple<S0,...,SM> >
   * and exports tuple<T0,..,TN,S0,...,SM>.
   *
   * \tparam TupleTuple A tuple of tuples
   */
  template<class TupleTuple>
  struct FlattenTuple
  {
    //! Result of the flatten operation
    typedef typename ReduceTuple< JoinTuples, TupleTuple>::type type;
  };

}

#endif
