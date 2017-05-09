// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TUPLE_UTILITY_HH
#define DUNE_TUPLE_UTILITY_HH

#include <cstddef>
#include <tuple>
#include <type_traits>

#include <dune/common/deprecated.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/std/type_traits.hh>
#include <dune/common/std/utility.hh>

namespace Dune {

  /** @addtogroup TupleUtilities
   *
   * @{
   */

  /**
   * @file
   * @brief Contains utility classes which can be used with std::tuple.
   */

  template<class T>
  struct TupleAccessTraits
  {
    typedef typename std::add_const<T>::type& ConstType;
    typedef T& NonConstType;
    typedef const typename std::remove_const<T>::type& ParameterType;
  };

  template<class T>
  struct TupleAccessTraits<T*>
  {
    typedef typename std::add_const<T>::type* ConstType;
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

  /**
   * @brief A helper template that initializes a std::tuple consisting of pointers
   * to nullptr.
   *
   * A std::tuple of nullptr may be useful when you use a std::tuple of pointers
   * in a class which you can only initialise in a later stage.
   */
  template<class T>
  struct NullPointerInitialiser;

  template<class... Args>
  struct NullPointerInitialiser<std::tuple<Args...> >
  {
    typedef std::tuple<Args...> ResultType;
    static ResultType apply()
    {
      return ResultType(static_cast<Args>(nullptr)...);
    }
  };

  /**
   * @brief Helper template to clone the type definition of a std::tuple with the
   * storage types replaced by a user-defined rule.
   *
   * Suppose all storage types A_i in a std::tuple define a type A_i::B. You can
   * build up a pair consisting of the types defined by A_i::B in the following
   * way:
   *
   * \code
   * template <class A>
   * struct MyEvaluator
   * {
   *   typedef typename A::B Type;
   * };
   *
   * typedef ForEachType<MyEvaluator, ATuple>::Type BTuple;
   * \endcode
   *
   * Here, MyEvaluator is a helper struct that extracts the correct type from
   * the storage types of the tuple defined by the tuple ATuple.
   *
   * \sa AddRefTypeEvaluator, AddPtrTypeEvaluator, genericTransformTuple(),
   *     and transformTuple().
   */
  template<template <class> class TE, class T>
  struct ForEachType;

  template<template <class> class TE, class... Args>
  struct ForEachType<TE, std::tuple<Args...> >
  {
    typedef std::tuple<typename TE<Args>::Type...> Type;
  };

#ifndef DOXYGEN
  template<class Tuple, class Functor, std::size_t... I>
  inline auto genericTransformTupleBackendImpl(Tuple& t, Functor& f, const Std::index_sequence<I...>& )
    -> std::tuple<decltype(f(std::get<I>(t)))...>
  {
    return std::tuple<decltype(f(std::get<I>(t)))...>(f(std::get<I>(t))...);
  }

  template<class... Args, class Functor>
  auto genericTransformTupleBackend(std::tuple<Args...>& t, Functor& f) ->
    decltype(genericTransformTupleBackendImpl(t, f,Std::index_sequence_for<Args...>{}))
  {
    return genericTransformTupleBackendImpl(t, f,Std::index_sequence_for<Args...>{});
  }

  template<class... Args, class Functor>
  auto genericTransformTupleBackend(const std::tuple<Args...>& t, Functor& f) ->
    decltype(genericTransformTupleBackendImpl(t, f, Std::index_sequence_for<Args...>{}))
  {
    return genericTransformTupleBackendImpl(t, f, Std::index_sequence_for<Args...>{});
  }
#endif

  /**
   * This function does for the value of a std::tuple what ForEachType does for the
   * type of a std::tuple: it transforms the value using a user-provided policy
   * functor.
   *
   * \param t The std::tuple value to transform.
   * \param f The functor to use to transform the values.
   *
   * The functor should have the following form:
   *
   * \code
   * struct Functor
   * {
   *   template<class>
   *   struct TypeEvaluator
   *   {
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
   *
   * The member class template \c TypeEvaluator should be a class template
   * suitable as the \c TypeEvaluator template parameter for ForEachType.  The
   * function call operator \c operator() is used to transform the value; only
   * the signatures of \c operator() which are actually used must be present.
   */
  template<class Tuple, class Functor>
  auto genericTransformTuple(Tuple&& t, Functor&& f) ->
    decltype(genericTransformTupleBackend(t, f))
  {
    return genericTransformTupleBackend(t, f);
  }

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
   *
   * \code
   * template <class T>
   * struct TypeEvaluator
   * {
   *   typedef T* Type;
   *   static Type apply(T& t, void* a0)
   *   {
   *     return t ? &t : static_cast<T*>(a0);
   *   }
   * };
   * \endcode
   *
   * In this example, for the value transformation, it takes a reference to a value
   * of type T and return the pointer to that value, unless the value evaluates to false
   * in boolean context.  If the value evaluates to false, it will instead return the
   * pointer from the extra argument.
   */
  template<template<class> class TE, class... Args>
  class TransformTupleFunctor
  {
    mutable std::tuple<Args&...> tup;

    template<class T, std::size_t... I>
    inline auto apply(T&& t, const Std::index_sequence<I...>& ) ->
      decltype(TE<T>::apply(t,std::get<I>(tup)...)) const
    {
      return TE<T>::apply(t,std::get<I>(tup)...);
    }

  public:
    template<class T>
    struct TypeEvaluator : public TE<T>
    {};

    TransformTupleFunctor(Args&&... args)
      : tup(args...)
    { }

    template<class T>
    inline auto operator()(T&& t) ->
      decltype(this->apply(t,Std::index_sequence_for<Args...>{})) const
    {
      return apply(t,Std::index_sequence_for<Args...>{});
    }
  };

  template<template<class> class TE, class... Args>
  TransformTupleFunctor<TE, Args...> makeTransformTupleFunctor(Args&&... args)
  {
    return TransformTupleFunctor<TE, Args...>(args...);
  }

  /**
   * This function provides functionality similar to genericTransformTuple(),
   * although less general and closer in spirit to ForEachType.
   *
   * \tparam TypeEvaluator Used as the \c TE template argument to
   *                       TransformTupleFunctor internally.
   * \tparam Tuple         Type of the std::tuple to transform.
   * \tparam Args          Types of extra argument to call the transformation
   *                       function with.
   *
   * \param orig Tuple value to be transformed.
   * \param args Extra arguments values to provide to the transformation
   *             function.
   *
   * The \c TypeEvaluator class template should be suitable as the \c TE
   * template argument for TransformTupleFunctor.  It has the following form
   * (an extension of the \c TypeEvaluator template argument of ForEachType):
   *
   * \code
   * template <class T>
   * struct TypeEvaluator
   * {
   *   typedef UserDefined Type;
   *
   *   template<class... Args>
   *   static Type apply(T& t, Args&... args);
   * };
   * \endcode
   *
   * \sa genericTransforTuple(), ForEachType, AddRefTypeEvaluator, and
   *     AddPtrTypeEvaluator.
   */
  template<template<class> class TypeEvaluator, class Tuple, class... Args>
  auto transformTuple(Tuple&& orig, Args&&... args) ->
    decltype(genericTransformTuple(orig, makeTransformTupleFunctor<TypeEvaluator>(args...)))
  {
    return genericTransformTuple(orig, makeTransformTupleFunctor<TypeEvaluator>(args...));
  }

  //! \c TypeEvaluator to turn a type \c T into a reference to \c T
  /**
   * This is suitable as the \c TypeEvaluator template parameter for
   * ForEachType and transformTuple().
   */
  template<class T>
  struct AddRefTypeEvaluator
  {
    typedef T& Type;
    static Type apply(T& t)
    {
      return t;
    }
  };

  //! \c TypeEvaluator to turn a type \c T into a pointer to \c T
  /**
   * This is suitable as the \c TypeEvaluator template parameter for
   * ForEachType and transformTuple().
   */
  template<class T>
  struct AddPtrTypeEvaluator
  {
    typedef typename std::remove_reference<T>::type* Type;
    static Type apply(T& t)
    {
      return &t;
    }
  };

  // Specialization, in case the type is already a reference
  template<class T>
  struct AddPtrTypeEvaluator<T&>
  {
    typedef typename std::remove_reference<T>::type* Type;
    static Type apply(T& t)
    {
      return &t;
    }
  };

  template<class Tuple>
  struct DUNE_DEPRECATED_MSG("Use Hybrid::forEach instead!") ForEachValue
  {
    ForEachValue(Tuple& t) :
      t_(t)
    {}

    template<class Functor>
    void apply(Functor& f) const
    {
      Hybrid::forEach(Std::make_index_sequence<std::tuple_size<Tuple>::value>{},
        [&](auto i){f.visit(std::get<i>(t_));});
    }

    Tuple& t_;
  };

  template<class Tuple1, class Tuple2>
  struct DUNE_DEPRECATED_MSG("Use Hybrid::forEach instead!") ForEachValuePair
  {
    ForEachValuePair(Tuple1& t1, Tuple2& t2) :
      t1_(t1),
      t2_(t2)
    {}

    template<class Functor>
    void apply(Functor& f)
    {
      Hybrid::forEach(Std::make_index_sequence<std::tuple_size<Tuple1>::value>{},
        [&](auto i){f.visit(std::get<i>(t1_), std::get<i>(t2_));});
    }

    Tuple1& t1_;
    Tuple2& t2_;
  };

  /**
   * @brief Type for reverse element access.
   *
   * Counterpart to ElementType for reverse element access.
   */
  template<int N, class Tuple>
  struct AtType
  {
    typedef typename std::tuple_element<std::tuple_size<Tuple>::value - N - 1, Tuple>::type Type;
  };

  /**
   * @brief Reverse element access.
   *
   * While Element<...> gives you the arguments beginning at the front of a
   * std::tuple, At<...> starts at the end, which may be more convenient, depending
   * on how you built your std::tuple.
   */
  template<int N>
  struct At
  {
    template<typename Tuple>
    static typename TupleAccessTraits<typename AtType<N, Tuple>::Type>::NonConstType
    get(Tuple& t)
    {
      return std::get<std::tuple_size<Tuple>::value - N - 1>(t);
    }

    template<typename Tuple>
    static typename TupleAccessTraits<typename AtType<N, Tuple>::Type>::ConstType
    get(const Tuple& t)
    {
      return std::get<std::tuple_size<Tuple>::value - N - 1>(t);
    }
  };

  /**
   * @brief Deletes all objects pointed to in a std::tuple of pointers.
   */
  template<class Tuple>
  struct PointerPairDeletor
  {
    template<typename... Ts>
    static void apply(std::tuple<Ts...>& t)
    {
      Hybrid::forEach(t,[&](auto&& ti){delete ti; ti=nullptr;});
    }
  };

  /**
   * @brief Finding the index of a certain type in a std::tuple
   *
   * \tparam Tuple     The std::tuple type to search in.
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
   *                   always be equal to the size of the std::tuple.
   *
   * This class can search for a type in std::tuple. It will apply the predicate
   * to each type in std::tuple in turn, and set its member constant \c value to
   * the index of the first type that was accepted by the predicate.  If none
   * of the types are accepted by the predicate, a static_assert is triggered.
   */
  template<class Tuple, template<class> class Predicate, std::size_t start = 0,
      std::size_t size = std::tuple_size<Tuple>::value>
  class FirstPredicateIndex :
    public std::conditional<Predicate<typename std::tuple_element<start,
                Tuple>::type>::value,
        std::integral_constant<std::size_t, start>,
        FirstPredicateIndex<Tuple, Predicate, start+1> >::type
  {
    static_assert(std::tuple_size<Tuple>::value == size, "The \"size\" "
                       "template parameter of FirstPredicateIndex is an "
                       "implementation detail and should never be set "
                       "explicitly!");
  };

#ifndef DOXYGEN
  template<class Tuple, template<class> class Predicate, std::size_t size>
  class FirstPredicateIndex<Tuple, Predicate, size, size>
  {
    static_assert(Std::to_false_type<Tuple>::value, "None of the std::tuple element "
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
  struct IsType
  {
    //! @brief The actual predicate
    template<class U>
    struct Predicate : public std::is_same<T, U> {};
  };

  /**
   * @brief Find the first occurrence of a type in a std::tuple
   *
   * \tparam Tuple The std::tuple type to search in.
   * \tparam T     Type to search for.
   * \tparam start First index to try.  This can be adjusted to skip leading
   *               std::tuple elements.
   *
   * This class can search for a particular type in std::tuple. It will check each
   * type in the std::tuple in turn, and set its member constant \c value to the
   * index of the first occurrence of type was found.  If the type was not
   * found, a static_assert is triggered.
   */
  template<class Tuple, class T, std::size_t start = 0>
  struct FirstTypeIndex :
    public FirstPredicateIndex<Tuple, IsType<T>::template Predicate, start>
  { };

  /**
   * \brief Helper template to append a type to a std::tuple
   *
   * \tparam Tuple The std::tuple type to extend
   * \tparam T     The type to be appended to the std::tuple
   */
  template<class Tuple, class T>
  struct PushBackTuple;

  template<class... Args, class T>
  struct PushBackTuple<typename std::tuple<Args...>, T>
  {
    typedef typename std::tuple<Args..., T> type;
  };

  /**
   * \brief Helper template to prepend a type to a std::tuple
   *
   * \tparam Tuple The std::tuple type to extend
   * \tparam T     The type to be prepended to the std::tuple
   */
  template<class Tuple, class T>
  struct PushFrontTuple;

  template<class... Args, class T>
  struct PushFrontTuple<typename std::tuple<Args...>, T>
  {
    typedef typename std::tuple<T, Args...> type;
  };

  /**
   * \brief Apply reduce with meta binary function to template
   *
   * For a tuple\<T0,T1,...,TN-1,TN,...\> the exported result is
   *
   * F\< ... F\< F\< F\<Seed,T0\>\::type, T1\>\::type, T2\>\::type,  ... TN-1\>\::type
   *
   * \tparam F Binary meta function
   * \tparam Tuple Apply reduce operation to this std::tuple
   * \tparam Seed Initial value for reduce operation
   * \tparam N Reduce the first N std::tuple elements
   */
  template<
      template <class, class> class F,
      class Tuple,
      class Seed=std::tuple<>,
      int N=std::tuple_size<Tuple>::value>
  struct ReduceTuple
  {
    typedef typename ReduceTuple<F, Tuple, Seed, N-1>::type Accumulated;
    typedef typename std::tuple_element<N-1, Tuple>::type Value;

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
   * \tparam Tuple Apply reduce operation to this std::tuple
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
   * \brief Join two std::tuple's
   *
   * For Head=std::tuple<T0,...,TN> and Tail=std::tuple<S0,...,SM>
   * the exported result is std::tuple<T0,..,TN,S0,...,SM>.
   *
   * \tparam Head Head of resulting std::tuple
   * \tparam Tail Tail of resulting std::tuple
   */
  template<class Head, class Tail>
  struct JoinTuples
  {
    //! Result of the join operation
    typedef typename ReduceTuple<PushBackTuple, Tail, Head>::type type;
  };

  /**
   * \brief Flatten a std::tuple of std::tuple's
   *
   * This flattens a std::tuple of tuples std::tuple<std::tuple<T0,...,TN>, std::tuple<S0,...,SM> >
   * and exports std::tuple<T0,..,TN,S0,...,SM>.
   *
   * \tparam TupleTuple A std::tuple of std::tuple's
   */
  template<class Tuple>
  struct FlattenTuple
  {
    //! Result of the flatten operation
    typedef typename ReduceTuple<JoinTuples, Tuple>::type type;
  };

  /** }@ */
}

#endif
