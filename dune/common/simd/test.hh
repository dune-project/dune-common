#ifndef DUNE_COMMON_SIMD_TEST_HH
#define DUNE_COMMON_SIMD_TEST_HH

/** @file
 *  @brief Common tests for simd abstraction implementations
 *
 * This file is an interface header and may be included without restrictions.
 */

#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <utility>

#include <dune/common/classname.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/simd/simd.hh>
#include <dune/common/std/type_traits.hh>
#include <dune/common/typelist.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/unused.hh>

namespace Dune {
  namespace Simd {

    namespace Impl {

      template<class Expr, class SFINAE = void>
      struct CanCall; // not defined unless Expr has the form Op(Args...)
      template<class Op, class... Args, class SFINAE>
      struct CanCall<Op(Args...), SFINAE> : std::false_type {};
      template<class Op, class... Args>
      struct CanCall<Op(Args...), void_t<std::result_of_t<Op(Args...)> > >
        : std::true_type
      {};

      template<class T, class SFINAE = void>
      struct LessThenComparable : std::false_type {};
      template<class T>
      struct LessThenComparable<T, void_t<decltype(std::declval<T>()
                                                   < std::declval<T>())> > :
        std::true_type
      {};

      template<class Dst, class Src>
      struct CopyConstHelper
      {
        using type = Dst;
      };
      template<class Dst, class Src>
      struct CopyConstHelper<Dst, const Src>
      {
        using type = std::add_const_t<Dst>;
      };

      template<class Dst, class Src>
      struct CopyVolatileHelper
      {
        using type = Dst;
      };
      template<class Dst, class Src>
      struct CopyVolatileHelper<Dst, volatile Src>
      {
        using type = std::add_volatile_t<Dst>;
      };

      template<class Dst, class Src>
      struct CopyReferenceHelper
      {
        using type = Dst;
      };
      template<class Dst, class Src>
      struct CopyReferenceHelper<Dst, Src&>
      {
        using type = std::add_lvalue_reference_t<Dst>;
      };

      template<class Dst, class Src>
      struct CopyReferenceHelper<Dst, Src&&>
      {
        using type = std::add_rvalue_reference_t<Dst>;
      };

      template<class Dst, class Src>
      using CopyRefQual = typename CopyReferenceHelper<
        typename CopyVolatileHelper<
          typename CopyConstHelper<
            std::decay_t<Dst>,
            std::remove_reference_t<Src>
            >::type,
          std::remove_reference_t<Src>
          >::type,
        Src
        >::type;

    } // namespace Impl

    class UnitTest {
      bool good_ = true;
      std::ostream &log_ = std::cerr;
      // records the types for which checks have started running to avoid
      // infinite recursion
      std::unordered_set<std::type_index> seen_;
      // Same for the extra checks for mask types, although here we only avoid
      // running checks more than once
      std::unordered_set<std::type_index> indexSeen_;

      // Same for the extra checks for mask types, although here we only avoid
      // running checks more than once
      std::unordered_set<std::type_index> maskSeen_;

      ////////////////////////////////////////////////////////////////////////
      //
      //  Helper functions
      //

      void complain(const char *file, int line, const char *func,
                    const char *expr);

      void complain(const char *file, int line, const char *func,
                    const std::string &opname, const char *expr);

      // This macro is defined only within this file, do not use anywhere
      // else.  Doing the actual printing in an external function dramatically
      // reduces memory use during compilation.  Defined in such a way that
      // the call will only happen for failed checks.
#define DUNE_SIMD_CHECK(expr)                                           \
      ((expr) ? void() : complain(__FILE__, __LINE__, __func__, #expr))

      // the function using this macro must define a way to compute the
      // operator name in DUNE_SIMD_OPNAME
#define DUNE_SIMD_CHECK_OP(expr)                                \
      ((expr) ? void() : complain(__FILE__, __LINE__, __func__, \
                                  DUNE_SIMD_OPNAME, #expr))

      // "cast" into a prvalue
      template<class T>
      static T prvalue(T t)
      {
        return t;
      }

      // whether the vector is 42 in all lanes
      template<class V>
      static bool is42(const V &v)
      {
        bool good = true;

        for(std::size_t l = 0; l < lanes(v); ++l)
          // need to cast in case we have a mask type
          good &= (lane(l, v) == Scalar<V>(42));

        return good;
      }

      // make a vector that contains the sequence { 1, 2, ... }
      template<class V>
      static V make123()
      {
        V vec;
        for(std::size_t l = 0; l < lanes(vec); ++l)
          lane(l, vec) = l + 1;
        return vec;
      }

      // whether the vector contains the sequence { 1, 2, ... }
      template<class V>
      static bool is123(const V &v)
      {
        bool good = true;

        for(std::size_t l = 0; l < lanes(v); ++l)
          // need to cast in case we have a mask type
          good &= (lane(l, v) == Scalar<V>(l+1));

        return good;
      }

      template<class V>
      static V leftVector()
      {
        V res;
        for(std::size_t l = 0; l < lanes(res); ++l)
          lane(l, res) = Scalar<V>(l+1);
        return res;
      }

      template<class V>
      static V rightVector()
      {
        V res;
        for(std::size_t l = 0; l < lanes(res); ++l)
          // do not exceed number of bits in char (for shifts)
          // avoid 0 (for / and %)
          lane(l, res) = Scalar<V>((l)%7+1);
        return res;
      }

      template<class T>
      static T leftScalar()
      {
        return T(42);
      }

      template<class T>
      static T rightScalar()
      {
        // do not exceed number of bits in char (for shifts)
        // avoid 0 (for / and %)
        return T(5);
      }

      template<class Op, class... Args>
      using ScalarResult =
        decltype(std::declval<Op>().scalar(std::declval<Args>()...));
      template<class Call>
      using CanCall = Impl::CanCall<Call>;

      template<class Dst, class Src>
      using CopyRefQual = Impl::CopyRefQual<Dst, Src>;

      //////////////////////////////////////////////////////////////////////
      //
      // Check associated types
      //

      template<class V>
      void checkScalar()
      {
        // check that the type Scalar<V> exists
        using T = Scalar<V>;

        static_assert(std::is_same<T, std::decay_t<T> >::value, "Scalar types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        T DUNE_UNUSED a{};
      }

      template<class V>
      void checkIndexOf()
      {
        // check that the type Scalar<V> exists
        using I = Index<V>;
        log_ << "Index type of " << className<V>() << " is " << className<I>()
             << std::endl;


        static_assert(std::is_same<I, std::decay_t<I> >::value, "Index types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        static_assert(lanes<V>() == lanes<I>(), "Index types must have the "
                      "same number of lanes as the original vector types");

        checkIndex<I>();
      }

      template<class V>
      void checkMaskOf()
      {
        // check that the type Scalar<V> exists
        using M = Mask<V>;
        log_ << "Mask type of " << className<V>() << " is " << className<M>()
             << std::endl;

        static_assert(lanes<V>() == lanes<M>(), "Mask types must have the "
                      "same number of lanes as the original vector types");

        checkMask<M>();
      }

      //////////////////////////////////////////////////////////////////////
      //
      //  Fundamental checks
      //

      template<class V>
      void checkLanes()
      {
        // check lanes
        static_assert(std::is_same<std::size_t, decltype(lanes<V>())>::value,
                      "return type of lanes<V>() should be std::size_t");
        static_assert(std::is_same<std::size_t, decltype(lanes(V{}))>::value,
                      "return type of lanes(V{}) should be std::size_t");

        // the result of lanes<V>() must be constexpr
        constexpr auto DUNE_UNUSED size = lanes<V>();
        // but the result of lanes(vec) does not need to be constexpr
        DUNE_SIMD_CHECK(lanes<V>() == lanes(V{}));
      }

      template<class V>
      void checkDefaultConstruct()
      {
        { V DUNE_UNUSED vec;      }
        { V DUNE_UNUSED vec{};    }
        { V DUNE_UNUSED vec = {}; }
      }

      template<class V>
      void checkLane()
      {
        V vec;
        // check lane() on mutable lvalues
        for(std::size_t l = 0; l < lanes(vec); ++l)
          lane(l, vec) = l + 1;
        for(std::size_t l = 0; l < lanes(vec); ++l)
          DUNE_SIMD_CHECK(lane(l, vec) == Scalar<V>(l + 1));
        using MLRes = decltype(lane(0, vec));
        static_assert(std::is_same<MLRes, Scalar<V>&>::value ||
                      std::is_same<MLRes, std::decay_t<MLRes> >::value,
                      "Result of lane() on a mutable lvalue vector must "
                      "either be a mutable reference to a scalar of that "
                      "vector or a proxy object (which itself may not be a "
                      "reference nor const).");

        // check lane() on const lvalues
        const V &vec2 = vec;
        for(std::size_t l = 0; l < lanes(vec); ++l)
          DUNE_SIMD_CHECK(lane(l, vec2) == Scalar<V>(l + 1));
        using CLRes = decltype(lane(0, vec2));
        static_assert(std::is_same<CLRes, const Scalar<V>&>::value ||
                      std::is_same<CLRes, std::decay_t<CLRes> >::value,
                      "Result of lane() on a const lvalue vector must "
                      "either be a const lvalue reference to a scalar of that "
                      "vector or a proxy object (which itself may not be a "
                      "reference nor const).");
        static_assert(!std::is_assignable<CLRes, Scalar<V> >::value,
                      "Result of lane() on a const lvalue vector must not be "
                      "assignable from a scalar.");

        // check lane() on rvalues
        for(std::size_t l = 0; l < lanes(vec); ++l)
          DUNE_SIMD_CHECK(lane(l, prvalue(vec)) == Scalar<V>(l + 1));
        using RRes = decltype(lane(0, prvalue(vec)));
        // TODO: do we really want to allow Scalar<V>&& here?  If we allow it,
        // then `auto &&res = lane(0, vec*vec);` creates a dangling reference,
        // and the scalar (and even the vector types) are small enough to be
        // passed in registers anyway.  On the other hand, the only comparable
        // accessor function in the standard library that I can think of is
        // std::get(), and that does return an rvalue reference in this
        // situation.  However, that cannot assume anything about the size of
        // the returned types.
        static_assert(std::is_same<RRes, Scalar<V>  >::value ||
                      std::is_same<RRes, Scalar<V>&&>::value,
                      "Result of lane() on a rvalue vector V must be "
                      "Scalar<V> or Scalar<V>&&.");
        // Can't assert non-assignable, fails for any typical class,
        // e.g. std::complex<>.  Would need to return const Scalar<V> or const
        // Scalar<V>&&, which would inhibit moving from the return value.
        // static_assert(!std::is_assignable<RRes, Scalar<V> >::value,
        //               "Result of lane() on a rvalue vector must not be "
        //               "assignable from a scalar.");
      }

      // check non-default constructors
      template<class V>
      void checkCopyMoveConstruct()
      {
        // elided copy/move constructors
        { V vec   (make123<V>()); DUNE_SIMD_CHECK(is123(vec)); }
        { V vec =  make123<V>() ; DUNE_SIMD_CHECK(is123(vec)); }
        { V vec   {make123<V>()}; DUNE_SIMD_CHECK(is123(vec)); }
        { V vec = {make123<V>()}; DUNE_SIMD_CHECK(is123(vec)); }

        // copy constructors
        {       V ref(make123<V>());     V vec   (ref);
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        {       V ref(make123<V>());     V vec =  ref ;
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        {       V ref(make123<V>());     V vec   {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        {       V ref(make123<V>());     V vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        { const V ref(make123<V>());     V vec   (ref);
          DUNE_SIMD_CHECK(is123(vec)); }
        { const V ref(make123<V>());     V vec =  ref ;
          DUNE_SIMD_CHECK(is123(vec)); }
        { const V ref(make123<V>());     V vec   {ref};
          DUNE_SIMD_CHECK(is123(vec)); }
        { const V ref(make123<V>());     V vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); }

        // move constructors
        { V ref(make123<V>());           V vec   (std::move(ref));
          DUNE_SIMD_CHECK(is123(vec)); }
        { V ref(make123<V>());           V vec =  std::move(ref) ;
          DUNE_SIMD_CHECK(is123(vec)); }
        { V ref(make123<V>());           V vec   {std::move(ref)};
          DUNE_SIMD_CHECK(is123(vec)); }
        { V ref(make123<V>());           V vec = {std::move(ref)};
          DUNE_SIMD_CHECK(is123(vec)); }
      }

      template<class V>
      void checkBroadcastVectorConstruct()
      {
        // broadcast copy constructors
        {       Scalar<V> ref = 42;      V vec   (ref);
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        {       Scalar<V> ref = 42;      V vec =  ref ;
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        // {       Scalar<V> ref = 42;      V vec   {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        // {       Scalar<V> ref = 42;      V vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        { const Scalar<V> ref = 42;      V vec   (ref);
          DUNE_SIMD_CHECK(is42(vec)); }
        { const Scalar<V> ref = 42;      V vec =  ref ;
          DUNE_SIMD_CHECK(is42(vec)); }
        // { const Scalar<V> ref = 42;      V vec   {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); }
        // { const Scalar<V> ref = 42;      V vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); }

        // broadcast move constructors
        { Scalar<V> ref = 42;            V vec   (std::move(ref));
          DUNE_SIMD_CHECK(is42(vec)); }
        { Scalar<V> ref = 42;            V vec =  std::move(ref) ;
          DUNE_SIMD_CHECK(is42(vec)); }
        // { Scalar<V> ref = 42;            V vec   {std::move(ref)};
        //   DUNE_SIMD_CHECK(is42(vec)); }
        // { Scalar<V> ref = 42;            V vec = {std::move(ref)};
        //   DUNE_SIMD_CHECK(is42(vec)); }
      }

      template<class V>
      void checkBroadcastMaskConstruct()
      {
        // broadcast copy constructors
        {       Scalar<V> ref = 42;      V vec   (ref);
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        // {       Scalar<V> ref = 42;      V vec =  ref ;
        //   DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        {       Scalar<V> ref = 42;      V vec   {ref};
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        // {       Scalar<V> ref = 42;      V vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        { const Scalar<V> ref = 42;      V vec   (ref);
          DUNE_SIMD_CHECK(is42(vec)); }
        // { const Scalar<V> ref = 42;      V vec =  ref ;
        //   DUNE_SIMD_CHECK(is42(vec)); }
        { const Scalar<V> ref = 42;      V vec   {ref};
          DUNE_SIMD_CHECK(is42(vec)); }
        // { const Scalar<V> ref = 42;      V vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); }

        // broadcast move constructors
        { Scalar<V> ref = 42;            V vec   (std::move(ref));
          DUNE_SIMD_CHECK(is42(vec)); }
        // { Scalar<V> ref = 42;            V vec =  std::move(ref) ;
        //   DUNE_SIMD_CHECK(is42(vec)); }
        { Scalar<V> ref = 42;            V vec   {std::move(ref)};
          DUNE_SIMD_CHECK(is42(vec)); }
        // { Scalar<V> ref = 42;            V vec = {std::move(ref)};
        //   DUNE_SIMD_CHECK(is42(vec)); }
      }

      template<class V>
      void checkBracedAssign()
      {
        // copy assignment
        { V ref = make123<V>();       V vec; vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        { const V ref = make123<V>(); V vec; vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }

        // move assignment
        { V vec; vec = {make123<V>()}; DUNE_SIMD_CHECK(is123(vec)); }
      }

      template<class V>
      void checkBracedBroadcastAssign()
      {
        // nothing works here
        // // broadcast copy assignment
        // { Scalar<V> ref = 42;       V vec; vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        // { const Scalar<V> ref = 42; V vec; vec = {ref};
        //   DUNE_SIMD_CHECK(is42(vec)); }

        // // broadcast move assignment
        // { Scalar<V> ref = 42; V vec; vec = {std::move(ref)};
        //   DUNE_SIMD_CHECK(is42(vec)); }
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for unary operators
      //

#define DUNE_SIMD_POSTFIX_OP(NAME, SYMBOL)              \
      struct OpPostfix##NAME                            \
      {                                                 \
        template<class V>                               \
        auto operator()(V&& v) const                    \
          -> decltype(std::forward<V>(v) SYMBOL)        \
        {                                               \
          return std::forward<V>(v) SYMBOL;             \
        }                                               \
      }

#define DUNE_SIMD_PREFIX_OP(NAME, SYMBOL)               \
      struct OpPrefix##NAME                             \
      {                                                 \
        template<class V>                               \
        auto operator()(V&& v) const                    \
          -> decltype(SYMBOL std::forward<V>(v))        \
        {                                               \
          return SYMBOL std::forward<V>(v);             \
        }                                               \
      }

      DUNE_SIMD_POSTFIX_OP(Decrement,        -- );
      DUNE_SIMD_POSTFIX_OP(Increment,        ++ );

      DUNE_SIMD_PREFIX_OP (Decrement,        -- );
      DUNE_SIMD_PREFIX_OP (Increment,        ++ );

      DUNE_SIMD_PREFIX_OP (Plus,             +  );
      DUNE_SIMD_PREFIX_OP (Minus,            -  );
      DUNE_SIMD_PREFIX_OP (LogicNot,         !  );
      // Do not warn about ~ being applied to bool.  (1) Yes, doing that is
      // weird, but we do want to test the weird stuff too.  (2) It avoids
      // running into <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82040> on
      // g++-7.0 through 7.2.  Also, ignore -Wpragmas to not warn about an
      // unknown -Wbool-operation on compilers that do not know that option.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wbool-operation"
      DUNE_SIMD_PREFIX_OP (BitNot,           ~  );
#pragma GCC diagnostic pop

#undef DUNE_SIMD_POSTFIX_OP
#undef DUNE_SIMD_PREFIX_OP

      template<class V, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkUnaryOpV(Op op)
      {
#define DUNE_SIMD_OPNAME (className<Op(V)>())
        // arguments
        auto val = leftVector<std::decay_t<V>>();

        // copy the arguments in case V is a references
        auto arg = val;
        auto &&result = op(static_cast<V>(arg));
        using T = Scalar<std::decay_t<decltype(result)> >;
        for(std::size_t l = 0; l < lanes(val); ++l)
        {
          // `op` might promote the argument.  This is a problem if the
          // argument of the operation on the right of the `==` is
          // e.g. `(unsigned short)1` and the operation is e.g. unary `-`.
          // Then the argument is promoted to `int` before applying the
          // negation, and the result is `(int)-1`.  However, the left side of
          // the `==` is still `(unsigned short)-1`, which typically is the
          // same as `(unsigned short)65535`.  The `==` promotes the left side
          // before comparing, so that becomes `(int)65535`.  It will then
          // compare `(int)65535` and `(int)-1` and rightly declare them to be
          // not equal.

          // To work around this, we explicitly convert the right side of the
          // `==` to the scalar type before comparing.
          DUNE_SIMD_CHECK_OP
            (lane(l, result)
               == static_cast<T>(op(lane(l, static_cast<V>(val)))));
        }
        // op might modify val, verify that any such modification also happens
        // in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V> >(); ++l)
          DUNE_SIMD_CHECK_OP(lane(l, val) == lane(l, arg));
#undef DUNE_SIMD_OPNAME
      }

      template<class V, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkUnaryOpV(Op op)
      {
        // log_ << "No " << className<Op(decltype(lane(0, std::declval<V>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V)>() << std::endl;
      }

      template<class V, class Op>
      void checkUnaryOpsV(Op op)
      {
        checkUnaryOpV<V&>(op);
        checkUnaryOpV<const V&>(op);
        checkUnaryOpV<V&&>(op);
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for binary operators
      //

#define DUNE_SIMD_INFIX_OP(NAME, SYMBOL)                               \
      struct OpInfix##NAME                                             \
      {                                                                 \
        template<class V1, class V2>                                    \
        decltype(auto) vector(V1&& v1, V2&& v2) const                   \
        {                                                               \
          return std::forward<V1>(v1) SYMBOL std::forward<V2>(v2);      \
        }                                                               \
        template<class S1, class S2>                                    \
        auto scalar(S1&& s1, S2&& s2) const                             \
          -> decltype(std::forward<S1>(s1) SYMBOL std::forward<S2>(s2)) \
        {                                                               \
          return std::forward<S1>(s1) SYMBOL std::forward<S2>(s2);      \
        }                                                               \
      }

      // for assign ops, accept only non-const lvalue arguments for scalars.
      // This is needed class scalars (e.g. std::complex) because non-const
      // class rvalues are actually usually assignable.  Though that
      // assignment happens to a temporary, and thus is lost.  Except that the
      // tests would bind the result of the assignment to a reference.  And
      // because that result is returned from a function by reference, even
      // though it is a temporary passed as an argument to that function,
      // accessing the result later is undefined behaviour.
#define DUNE_SIMD_ASSIGN_OP(NAME, SYMBOL)                               \
      struct OpInfix##NAME                                              \
      {                                                                 \
        template<class V1, class V2>                                    \
        decltype(auto) vector(V1&& v1, V2&& v2) const                   \
        {                                                               \
          return std::forward<V1>(v1) SYMBOL std::forward<V2>(v2);      \
        }                                                               \
        template<class S1, class S2>                                    \
        auto scalar(S1& s1, S2&& s2) const                              \
          -> decltype(s1 SYMBOL std::forward<S2>(s2))                   \
        {                                                               \
          return s1 SYMBOL std::forward<S2>(s2);                        \
        }                                                               \
      }

#define DUNE_SIMD_REPL_OP(NAME, REPLFN, SYMBOL)                         \
      struct OpInfix##NAME                                              \
      {                                                                 \
        template<class V1, class V2>                                    \
        decltype(auto) vector(V1&& v1, V2&& v2) const                   \
        {                                                               \
          return Simd::REPLFN(std::forward<V1>(v1), std::forward<V2>(v2)); \
        }                                                               \
        template<class S1, class S2>                                    \
        auto scalar(S1&& s1, S2&& s2) const                             \
          -> decltype(std::forward<S1>(s1) SYMBOL std::forward<S2>(s2)) \
        {                                                               \
          return std::forward<S1>(s1) SYMBOL std::forward<S2>(s2);      \
        }                                                               \
      }

      DUNE_SIMD_INFIX_OP(Mul,              *  );
      DUNE_SIMD_INFIX_OP(Div,              /  );
      DUNE_SIMD_INFIX_OP(Remainder,        %  );

      DUNE_SIMD_INFIX_OP(Plus,             +  );
      DUNE_SIMD_INFIX_OP(Minus,            -  );

      DUNE_SIMD_INFIX_OP(LeftShift,        << );
      DUNE_SIMD_INFIX_OP(RightShift,       >> );

      DUNE_SIMD_INFIX_OP(Less,             <  );
      DUNE_SIMD_INFIX_OP(Greater,          >  );
      DUNE_SIMD_INFIX_OP(LessEqual,        <= );
      DUNE_SIMD_INFIX_OP(GreaterEqual,     >= );

      DUNE_SIMD_INFIX_OP(Equal,            == );
      DUNE_SIMD_INFIX_OP(NotEqual,         != );

      DUNE_SIMD_INFIX_OP(BitAnd,           &  );
      DUNE_SIMD_INFIX_OP(BitXor,           ^  );
      DUNE_SIMD_INFIX_OP(BitOr,            |  );

      // Those are not supported in any meaningful way by vectorclass
      // We need to test replacement functions maskAnd() and maskOr() instead.
      DUNE_SIMD_REPL_OP(LogicAnd, maskAnd, && );
      DUNE_SIMD_REPL_OP(LogicOr,  maskOr,  || );

      DUNE_SIMD_ASSIGN_OP(Assign,           =  );
      DUNE_SIMD_ASSIGN_OP(AssignMul,        *= );
      DUNE_SIMD_ASSIGN_OP(AssignDiv,        /= );
      DUNE_SIMD_ASSIGN_OP(AssignRemainder,  %= );
      DUNE_SIMD_ASSIGN_OP(AssignPlus,       += );
      DUNE_SIMD_ASSIGN_OP(AssignMinus,      -= );
      DUNE_SIMD_ASSIGN_OP(AssignLeftShift,  <<=);
      DUNE_SIMD_ASSIGN_OP(AssignRightShift, >>=);
      DUNE_SIMD_ASSIGN_OP(AssignAnd,        &= );
      DUNE_SIMD_ASSIGN_OP(AssignXor,        ^= );
      DUNE_SIMD_ASSIGN_OP(AssignOr,         |= );

#undef DUNE_SIMD_INFIX_OP
#undef DUNE_SIMD_REPL_OP
#undef DUNE_SIMD_ASSIGN_OP

      // just used as a tag
      struct OpInfixComma {};

      template<class T1, class T2>
      void checkCommaOp(std::decay_t<T1> val1, std::decay_t<T2> val2)
      {
#define DUNE_SIMD_OPNAME (className<OpInfixComma(T1, T2)>())
        static_assert(std::is_same<decltype((std::declval<T1>(),
                                             std::declval<T2>())), T2>::value,
                      "Type and value category of the comma operator must "
                      "match that of the second operand");

        // copy the arguments in case T1 or T2 are references
        auto arg1 = val1;
        auto arg2 = val2;
        // Do not warn that the left side of the comma operator is unused.
        // Seems to work for g++-4.9 and clang++-3.8.  Appears to be harmless
        // for icpc (14 and 17), and icpc does not seem to issue a warning
        // anyway.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
        auto &&result = (static_cast<T1>(arg1),
                         static_cast<T2>(arg2));
#pragma GCC diagnostic pop
        if(std::is_reference<T2>::value)
        {
          // comma should return the same object as the second argument for
          // lvalues and xvalues
          DUNE_SIMD_CHECK_OP(&result == &arg2);
          // it should not modify any arguments
          DUNE_SIMD_CHECK_OP(allTrue(val1 == arg1));
          DUNE_SIMD_CHECK_OP(allTrue(val2 == arg2));
        }
        else
        {
          // comma should return the same value as the second argument for
          // prvalues
          DUNE_SIMD_CHECK_OP(allTrue(result == arg2));
          // it should not modify any arguments
          DUNE_SIMD_CHECK_OP(allTrue(val1 == arg1));
          // second argument is a prvalue, any modifications happen to a
          // temporary and we can't detect them
        }
#undef DUNE_SIMD_OPNAME
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for vector-vector binary operations
      //

      // We check the following candidate operation
      //
      //   vopres = vop1 @ vop2
      //
      // against the reference operation
      //
      //   arefres[l] = aref1[l] @ aref2[l]  foreach l
      //
      // v... variables are simd-vectors and a... variables are arrays.  The
      // operation may modify the operands, but if is does the modification
      // needs to happen in both the candidate and the reference.
      //
      // We do the following checks:
      // 1.  lanes(vopres)   == lanes(vop1)
      // 2.  lane(l, vopres) == arefres[l]  foreach l
      // 3.  lane(l, vop1)   == aref1[l]    foreach l
      // 4.  lane(l, vop2)   == aref2[l]    foreach l
      template<class V1, class V2, class Op>
      std::enable_if_t<
        Std::is_detected_v<ScalarResult, Op,
                           decltype(lane(0, std::declval<V1>())),
                           decltype(lane(0, std::declval<V2>()))> >
      checkBinaryOpVV(MetaType<V1>, MetaType<V2>, Op op)
      {
#define DUNE_SIMD_OPNAME (className<Op(V1, V2)>())
        static_assert(std::is_same<std::decay_t<V1>, std::decay_t<V2> >::value,
                      "Internal testsystem error: called with two types that "
                      "don't decay to the same thing");

        // reference arguments
        auto vref1 = leftVector<std::decay_t<V1>>();
        auto vref2 = rightVector<std::decay_t<V2>>();

        // candidate arguments
        auto vop1 = vref1;
        auto vop2 = vref2;

        // candidate operation
        auto &&vopres =
          op.vector(static_cast<V1>(vop1), static_cast<V2>(vop2));
        using VR = decltype(vopres);

        // check 1.  lanes(vopres)   == lanes(vop1)
        static_assert(lanes<std::decay_t<VR> >() == lanes<std::decay_t<V1> >(),
                      "The result must have the same number of lanes as the "
                      "operands.");

        // do the reference operation, and simultaneously
        // check 2.  lane(l, vopres) == arefres[l]  foreach l
        using T = Scalar<std::decay_t<VR> >;
        for(auto l : range(lanes(vopres)))
        {
          // see the lengthy comment in `checkUnaryOpV()` as to why the
          // `static_cast` around the `op()` is necessary
          DUNE_SIMD_CHECK_OP
            (lane(l, vopres)
               == static_cast<T>(op.scalar(lane(l, static_cast<V1>(vref1)),
                                           lane(l, static_cast<V2>(vref2)))));
        }

        // check 3.  lane(l, vop1)   == aref1[l]    foreach l
        for(auto l : range(lanes(vop1)))
          DUNE_SIMD_CHECK_OP(lane(l, vop1) == lane(l, vref1));

        // check 4.  lane(l, vop2)   == aref2[l]    foreach l
        for(auto l : range(lanes(vop2)))
          DUNE_SIMD_CHECK_OP(lane(l, vop2) == lane(l, vref2));

#undef DUNE_SIMD_OPNAME
      }

      template<class V1, class V2, class Op>
      std::enable_if_t<
        !Std::is_detected_v<ScalarResult, Op,
                            decltype(lane(0, std::declval<V1>())),
                            decltype(lane(0, std::declval<V2>()))> >
      checkBinaryOpVV(MetaType<V1>, MetaType<V2>, Op op)
      {
        // log_ << "No " << className<Op(decltype(lane(0, std::declval<V1>())),
        //                               decltype(lane(0, std::declval<V2>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V1, V2)>() << std::endl;
      }

      template<class V1, class V2>
      void checkBinaryOpVV(MetaType<V1>, MetaType<V2>, OpInfixComma)
      {
        static_assert(std::is_same<std::decay_t<V1>, std::decay_t<V2> >::value,
                      "Internal testsystem error: called with two types that "
                      "don't decay to the same thing");

        checkCommaOp<V1, V2>(leftVector<std::decay_t<V1>>(),
                             rightVector<std::decay_t<V2>>());
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for vector-scalar binary operations
      //

      // We check the following candidate operation
      //
      //   vopres = vop1 @ sop2
      //
      // against the reference operation
      //
      //   arefres[l] = aref1[l] @ sref2  foreach l
      //
      // v... variables are simd-vectors, a... variables are arrays, and
      // s... variables are scalars.  The operation may modify the left
      // operand, but if is does the modifications needs to happen in both the
      // candidate and the reference.
      //
      // We do the following checks:
      // 1.  lanes(vopres)   == lanes(vop1)
      // 2.  lane(l, vopres) == arefres[l]  foreach l
      // 3.  lane(l, vop1)   == aref1[l]    foreach l
      // 4.  sop2  is never modified
      // 5.  sref2 is never modified
      //
      // In fact, if the property "sref2 is never modified" is violated that
      // means the operation is unsuitable for an automatic broadcast of the
      // second operand and should not be checked.  There are no operations in
      // the standard where the second operand is modified like this, but
      // there are operations where the first operand is modified -- and this
      // check is used for thos ops as well by exchanging the first and second
      // argument below.

      template<class V1, class T2, class Op>
      std::enable_if_t<
        Std::is_detected_v<ScalarResult, Op,
                           decltype(lane(0, std::declval<V1>())), T2> >
      checkBinaryOpVS(MetaType<V1>, MetaType<T2>, Op op)
      {
#define DUNE_SIMD_OPNAME (className<Op(V1, T2)>())
        static_assert(std::is_same<Scalar<std::decay_t<V1> >,
                      std::decay_t<T2> >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        // initial values
        auto sinit2 = rightScalar<std::decay_t<T2>>();

        // reference arguments
        auto vref1 = leftVector<std::decay_t<V1>>();
        auto sref2 = sinit2;

        // candidate arguments
        auto vop1 = vref1;
        auto sop2 = sref2;

        // candidate operation
        auto &&vopres =
          op.vector(static_cast<V1>(vop1), static_cast<T2>(sop2));
        using VR = decltype(vopres);

        // check 1.  lanes(vopres)   == lanes(vop1)
        static_assert(lanes<std::decay_t<VR> >() == lanes<std::decay_t<V1> >(),
                      "The result must have the same number of lanes as the "
                      "operands.");

        // check 4.  sop2  is never modified
        DUNE_SIMD_CHECK_OP(sop2 == sinit2);

        // do the reference operation, and simultaneously check 2. and 5.
        using T = Scalar<std::decay_t<decltype(vopres)> >;
        for(auto l : range(lanes(vopres)))
        {
          // check 2.  lane(l, vopres) == arefres[l]  foreach l
          // see the lengthy comment in `checkUnaryOpV()` as to why the
          // `static_cast` around the `op()` is necessary
          DUNE_SIMD_CHECK_OP
            (lane(l, vopres)
               == static_cast<T>(op.scalar(lane(l, static_cast<V1>(vref1)),
                                                   static_cast<T2>(sref2) )));
          // check 5.  sref2 is never modified
          DUNE_SIMD_CHECK_OP(sref2 == sinit2);
        }

        // check 3.  lane(l, vop1)   == aref1[l]    foreach l
        for(auto l : range(lanes(vop1)))
          DUNE_SIMD_CHECK_OP(lane(l, vop1) == lane(l, vref1));

#undef DUNE_SIMD_OPNAME
      }

      template<class V1, class T2, class Op>
      std::enable_if_t<
        !Std::is_detected_v<ScalarResult, Op,
                            decltype(lane(0, std::declval<V1>())), T2> >
      checkBinaryOpVS(MetaType<V1>, MetaType<T2>, Op op)
      {
        // log_ << "No "
        //      << className<Op(decltype(lane(0, std::declval<V1>())), T2)>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V1, T2)>() << std::endl;
      }

      template<class V1, class T2>
      void checkBinaryOpVS(MetaType<V1>, MetaType<T2>, OpInfixComma)
      {
        static_assert(std::is_same<Scalar<std::decay_t<V1> >,
                      std::decay_t<T2> >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        checkCommaOp<V1, T2>(leftVector<std::decay_t<V1>>(),
                             rightScalar<std::decay_t<T2>>());
      }

      //////////////////////////////////////////////////////////////////////
      //
      // cross-check scalar-vector binary operations against vector-vector
      //

      // We check the following candidate operation
      //
      //   vopres = vop1 @ vop2,    where vop2 = broadcast(sref2)
      //
      // against the reference operation
      //
      //   vrefres = vref1 @ sref2
      //
      // v... variables are simd-vectors, a... variables are arrays, and
      // s... variables are scalars.
      //
      // We could check the following properties
      // 1.  lanes(vopres)   == lanes(vop1)
      // 2.  lane(l, vopres) == lane(l, vrefres)  foreach l
      // 3.  lane(l, vop1)   == lane(l, vref1)    foreach l
      // but these are given by checking the operation against the scalar
      // operation in the vector@vector and vector@scalar cases above.
      //
      // The only thing left to check is:
      // 4.  lane(l, vop2)  foreach l  is never modified

      template<class V1, class T2, class Op>
      std::enable_if_t<
        Std::is_detected_v<ScalarResult, Op,
                           decltype(lane(0, std::declval<V1>())), T2> >
      checkBinaryOpVVAgainstVS(MetaType<V1>, MetaType<T2>, Op op)
      {
#define DUNE_SIMD_OPNAME (className<Op(V1, T2)>())
        static_assert(std::is_same<Scalar<std::decay_t<V1> >,
                      std::decay_t<T2> >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        // initial values
        auto sinit2 = rightScalar<std::decay_t<T2>>();

        // reference arguments
        auto vop1 = leftVector<std::decay_t<V1>>();
        using V2 = CopyRefQual<V1, T2>;
        std::decay_t<V2> vop2(sinit2);

        // candidate operation
        op.vector(static_cast<V1>(vop1), static_cast<V2>(vop2));

        // 4.  lane(l, vop2)  foreach l  is never modified
        for(auto l : range(lanes(vop2)))
          DUNE_SIMD_CHECK_OP(lane(l, vop2) == sinit2);

#undef DUNE_SIMD_OPNAME
      }

      template<class V1, class T2, class Op>
      std::enable_if_t<
        !Std::is_detected_v<ScalarResult, Op,
                            decltype(lane(0, std::declval<V1>())), T2> >
      checkBinaryOpVVAgainstVS(MetaType<V1>, MetaType<T2>, Op op)
      {
        // log_ << "No "
        //      << className<Op(decltype(lane(0, std::declval<V1>())), T2)>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V1, T2)>() << std::endl;
      }

      template<class V1, class T2>
      void checkBinaryOpVVAgainstVS(MetaType<V1>, MetaType<T2>, OpInfixComma)
      { }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for scalar-vector binary operations
      //

      template<class Op>
      struct OpInfixSwappedArgs
      {
        Op orig;

        template<class V1, class V2>
        decltype(auto) vector(V1&& v1, V2&& v2) const
        {
          return orig.vector(std::forward<V2>(v2), std::forward<V1>(v1));
        }
        template<class S1, class S2>
        auto scalar(S1&& s1, S2&& s2) const
          -> decltype(orig.scalar(std::forward<S2>(s2), std::forward<S1>(s1)))
        {
          return orig.scalar(std::forward<S2>(s2), std::forward<S1>(s1));
        }
      };

      template<class T1, class V2, class Op>
      void checkBinaryOpSV(MetaType<T1> t1, MetaType<V2> v2, Op op)
      {
        checkBinaryOpVS(v2, t1, OpInfixSwappedArgs<Op>{op});
      }

      template<class T1, class V2>
      void checkBinaryOpSV(MetaType<T1>, MetaType<V2>, OpInfixComma)
      {
        static_assert(std::is_same<std::decay_t<T1>,
                      Scalar<std::decay_t<V2> > >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        checkCommaOp<T1, V2>(leftScalar<std::decay_t<T1>>(),
                             rightVector<std::decay_t<V2>>());
      }

      //////////////////////////////////////////////////////////////////////
      //
      // cross-check scalar-vector binary operations against vector-vector
      //

      // We check the following candidate operation
      //
      //   vopres = vop1 @ vop2,    where vop2 = broadcast(sref2)
      //
      // against the reference operation
      //
      //   vrefres = vref1 @ sref2
      //
      // v... variables are simd-vectors, a... variables are arrays, and
      // s... variables are scalars.
      //
      // We could check the following properties
      // 1.  lanes(vopres)   == lanes(vop1)
      // 2.  lane(l, vopres) == lane(l, vrefres)  foreach l
      // 3.  lane(l, vop1)   == lane(l, vref1)    foreach l
      // but these are given by checking the operation against the scalar
      // operation in the vector@vector and vector@scalar cases above.
      //
      // The only thing left to check is:
      // 4.  lane(l, vop2)  foreach l  is never modified

      template<class T1, class V2, class Op>
      void checkBinaryOpVVAgainstSV(MetaType<T1> t1, MetaType<V2> v2, Op op)
      {
        checkBinaryOpVVAgainstVS(v2, t1, OpInfixSwappedArgs<Op>{op});
      }

      template<class V1, class T2>
      void checkBinaryOpVVAgainstSV(MetaType<V1>, MetaType<T2>, OpInfixComma)
      { }

      //////////////////////////////////////////////////////////////////////
      //
      //  Invoke the checks for all combinations
      //

      template<class T1, class T2, bool condition, class Checker>
      void checkBinaryRefQual(Checker checker)
      {
        Hybrid::ifElse(std::integral_constant<bool, condition>{},
          [=] (auto id) {
            Hybrid::forEach(id(TypeList<T1&, const T1&, T1&&>{}),
              [=] (auto t1) {
                Hybrid::forEach(id(TypeList<T2&, const T2&, T2&&>{}),
                  [=] (auto t2) { id(checker)(t1, t2); });
              });
          });
      }

      template<class V, bool doSV, bool doVV, bool doVS, class Op>
      void checkBinaryOp(Op op)
      {
        checkBinaryRefQual<Scalar<V>, V, doSV>
          ([=](auto t1, auto t2) { this->checkBinaryOpSV(t1, t2, op); });
        checkBinaryRefQual<V, V, doVV>
          ([=](auto t1, auto t2) { this->checkBinaryOpVV(t1, t2, op); });
        checkBinaryRefQual<V, Scalar<V>, doVS>
          ([=](auto t1, auto t2) { this->checkBinaryOpVS(t1, t2, op); });

        // cross-check
        checkBinaryRefQual<Scalar<V>, V, doSV && doVV>
          ([=](auto t1, auto t2) {
            this->checkBinaryOpVVAgainstSV(t1, t2, op);
          });
        checkBinaryRefQual<V, Scalar<V>, doVV && doVS>
          ([=](auto t1, auto t2) {
            this->checkBinaryOpVVAgainstVS(t1, t2, op);
          });
      }

      static constexpr bool doCombo   = false;
      static constexpr bool doComboSV = true;
      static constexpr bool doComboVV = true;
      static constexpr bool doComboVS = true;

#define DUNE_SIMD_BINARY_OPCHECK(C1, C2, C3, NAME)      \
      checkBinaryOp<V, doCombo##C1, doCombo##C2, doCombo##C3>(Op##NAME{})

      template<class V>
      void checkVectorOps()
      {
        // postfix
        // checkUnaryOpsV<V>(OpPostfixDecrement{});
        // checkUnaryOpsV<V>(OpPostfixIncrement{});

        // prefix
        // checkUnaryOpsV<V>(OpPrefixDecrement{});
        // checkUnaryOpsV<V>(OpPrefixIncrement{});

        // checkUnaryOpsV<V>(OpPrefixPlus{});
        checkUnaryOpsV<V>(OpPrefixMinus{});
        checkUnaryOpsV<V>(OpPrefixLogicNot{});
        checkUnaryOpsV<V>(OpPrefixBitNot{});

        // binary
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixMul             );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixDiv             );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixRemainder       );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixPlus            );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixMinus           );

        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixLeftShift       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixRightShift      );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLess            );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixGreater         );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLessEqual       );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixGreaterEqual    );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixEqual           );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixNotEqual        );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitAnd          );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitXor          );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitOr           );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicAnd        );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicOr         );

        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssign          );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignMul       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignDiv       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignRemainder );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignPlus      );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignMinus     );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignLeftShift );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignRightShift);
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignAnd       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignXor       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignOr        );

        DUNE_SIMD_BINARY_OPCHECK(SV,   , VS, InfixComma           );
      }

      template<class V>
      void checkIndexOps()
      {
        // postfix
        // checkUnaryOpsV<V>(OpPostfixDecrement{});
        // checkUnaryOpsV<V>(OpPostfixIncrement{});

        // prefix
        // checkUnaryOpsV<V>(OpPrefixDecrement{});
        // checkUnaryOpsV<V>(OpPrefixIncrement{});

        // checkUnaryOpsV<V>(OpPrefixPlus{});
        checkUnaryOpsV<V>(OpPrefixMinus{});
        checkUnaryOpsV<V>(OpPrefixLogicNot{});
        checkUnaryOpsV<V>(OpPrefixBitNot{});

        // binary
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixMul             );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixDiv             );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixRemainder       );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixPlus            );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixMinus           );

        DUNE_SIMD_BINARY_OPCHECK(  ,   , VS, InfixLeftShift       );
        DUNE_SIMD_BINARY_OPCHECK(  ,   , VS, InfixRightShift      );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLess            );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixGreater         );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLessEqual       );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixGreaterEqual    );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixEqual           );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixNotEqual        );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitAnd          );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitXor          );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixBitOr           );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicAnd        );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicOr         );

        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssign          );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignMul       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignDiv       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignRemainder );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignPlus      );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignMinus     );
        DUNE_SIMD_BINARY_OPCHECK(  ,   , VS, InfixAssignLeftShift );
        DUNE_SIMD_BINARY_OPCHECK(  ,   , VS, InfixAssignRightShift);
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignAnd       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignXor       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV, VS, InfixAssignOr        );

        DUNE_SIMD_BINARY_OPCHECK(SV,   , VS, InfixComma           );
      }

      template<class V>
      void checkMaskOps()
      {
        // postfix
        // checkUnaryOpsV<V>(OpPostfixDecrement{});
        // clang deprecation warning if bool++ is tested
        // checkUnaryOpsV<V>(OpPostfixIncrement{});

        // prefix
        // checkUnaryOpsV<V>(OpPrefixDecrement{});
        // clang deprecation warning if ++bool is tested
        // checkUnaryOpsV<V>(OpPrefixIncrement{});

        // checkUnaryOpsV<V>(OpPrefixPlus{});
        // checkUnaryOpsV<V>(OpPrefixMinus{});
        checkUnaryOpsV<V>(OpPrefixLogicNot{});
        // checkUnaryOpsV<V>(OpPrefixBitNot{});

        // binary
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixMul             );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixDiv             );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixRemainder       );

        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixPlus            );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixMinus           );

        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixLeftShift       );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixRightShift      );

        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixLess            );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixGreater         );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixLessEqual       );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixGreaterEqual    );

        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixEqual           );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixNotEqual        );

        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixBitAnd          );
        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixBitXor          );
        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixBitOr           );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicAnd        );
        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixLogicOr         );

        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixAssign          );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignMul       );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignDiv       );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignRemainder );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignPlus      );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignMinus     );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignLeftShift );
        DUNE_SIMD_BINARY_OPCHECK(  ,   ,   , InfixAssignRightShift);
        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixAssignAnd       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixAssignXor       );
        DUNE_SIMD_BINARY_OPCHECK(  , VV,   , InfixAssignOr        );

        DUNE_SIMD_BINARY_OPCHECK(SV, VV, VS, InfixComma           );
      }

#undef DUNE_SIMD_BINARY_OPCHECK

      //////////////////////////////////////////////////////////////////////
      //
      // SIMD interface functions
      //

      template<class V>
      void checkAutoCopy()
      {
        using RValueResult = decltype(autoCopy(lane(0, std::declval<V>())));
        static_assert(std::is_same<RValueResult, Scalar<V> >::value,
                      "Result of autoCopy() must always be Scalar<V>");

        using MutableLValueResult =
          decltype(autoCopy(lane(0, std::declval<V&>())));
        static_assert(std::is_same<MutableLValueResult, Scalar<V> >::value,
                      "Result of autoCopy() must always be Scalar<V>");

        using ConstLValueResult =
          decltype(autoCopy(lane(0, std::declval<const V&>())));
        static_assert(std::is_same<ConstLValueResult, Scalar<V> >::value,
                      "Result of autoCopy() must always be Scalar<V>");

        V vec = make123<V>();
        for(std::size_t l = 0; l < lanes(vec); ++l)
          DUNE_SIMD_CHECK(autoCopy(lane(l, vec)) == Scalar<V>(l+1));
      }

      // may only be called for mask types
      template<class M>
      void checkBoolReductions()
      {
        M trueVec(true);

        // mutable lvalue
        DUNE_SIMD_CHECK(allTrue (static_cast<M&>(trueVec)) == true);
        DUNE_SIMD_CHECK(anyTrue (static_cast<M&>(trueVec)) == true);
        DUNE_SIMD_CHECK(allFalse(static_cast<M&>(trueVec)) == false);
        DUNE_SIMD_CHECK(anyFalse(static_cast<M&>(trueVec)) == false);

        // const lvalue
        DUNE_SIMD_CHECK(allTrue (static_cast<const M&>(trueVec)) == true);
        DUNE_SIMD_CHECK(anyTrue (static_cast<const M&>(trueVec)) == true);
        DUNE_SIMD_CHECK(allFalse(static_cast<const M&>(trueVec)) == false);
        DUNE_SIMD_CHECK(anyFalse(static_cast<const M&>(trueVec)) == false);

        // rvalue
        DUNE_SIMD_CHECK(allTrue (M(true)) == true);
        DUNE_SIMD_CHECK(anyTrue (M(true)) == true);
        DUNE_SIMD_CHECK(allFalse(M(true)) == false);
        DUNE_SIMD_CHECK(anyFalse(M(true)) == false);

        M falseVec(false);

        // mutable lvalue
        DUNE_SIMD_CHECK(allTrue (static_cast<M&>(falseVec)) == false);
        DUNE_SIMD_CHECK(anyTrue (static_cast<M&>(falseVec)) == false);
        DUNE_SIMD_CHECK(allFalse(static_cast<M&>(falseVec)) == true);
        DUNE_SIMD_CHECK(anyFalse(static_cast<M&>(falseVec)) == true);

        // const lvalue
        DUNE_SIMD_CHECK(allTrue (static_cast<const M&>(falseVec)) == false);
        DUNE_SIMD_CHECK(anyTrue (static_cast<const M&>(falseVec)) == false);
        DUNE_SIMD_CHECK(allFalse(static_cast<const M&>(falseVec)) == true);
        DUNE_SIMD_CHECK(anyFalse(static_cast<const M&>(falseVec)) == true);

        // rvalue
        DUNE_SIMD_CHECK(allTrue (M(false)) == false);
        DUNE_SIMD_CHECK(anyTrue (M(false)) == false);
        DUNE_SIMD_CHECK(allFalse(M(false)) == true);
        DUNE_SIMD_CHECK(anyFalse(M(false)) == true);

        M mixedVec;
        for(std::size_t l = 0; l < lanes(mixedVec); ++l)
          lane(l, mixedVec) = (l % 2);

        // mutable lvalue
        DUNE_SIMD_CHECK
          (allTrue (static_cast<M&>(mixedVec)) == false);
        DUNE_SIMD_CHECK
          (anyTrue (static_cast<M&>(mixedVec)) == (lanes<M>() > 1));
        DUNE_SIMD_CHECK
          (allFalse(static_cast<M&>(mixedVec)) == (lanes<M>() == 1));
        DUNE_SIMD_CHECK
          (anyFalse(static_cast<M&>(mixedVec)) == true);

        // const lvalue
        DUNE_SIMD_CHECK
          (allTrue (static_cast<const M&>(mixedVec)) == false);
        DUNE_SIMD_CHECK
          (anyTrue (static_cast<const M&>(mixedVec)) == (lanes<M>() > 1));
        DUNE_SIMD_CHECK
          (allFalse(static_cast<const M&>(mixedVec)) == (lanes<M>() == 1));
        DUNE_SIMD_CHECK
          (anyFalse(static_cast<const M&>(mixedVec)) == true);

        // rvalue
        DUNE_SIMD_CHECK(allTrue (M(mixedVec)) == false);
        DUNE_SIMD_CHECK(anyTrue (M(mixedVec)) == (lanes<M>() > 1));
        DUNE_SIMD_CHECK(allFalse(M(mixedVec)) == (lanes<M>() == 1));
        DUNE_SIMD_CHECK(anyFalse(M(mixedVec)) == true);
      }

      template<class V>
      void checkCond()
      {
        using M = Mask<V>;

        static_assert
          (std::is_same<decltype(cond(std::declval<M>(), std::declval<V>(),
                                      std::declval<V>())), V>::value,
           "The result of cond(M, V, V) should have exactly the type V");

        static_assert
          (std::is_same<decltype(cond(std::declval<const M&>(),
                                      std::declval<const V&>(),
                                      std::declval<const V&>())), V>::value,
           "The result of cond(const M&, const V&, const V&) should have "
           "exactly the type V");

        static_assert
          (std::is_same<decltype(cond(std::declval<M&>(), std::declval<V&>(),
                                      std::declval<V&>())), V>::value,
           "The result of cond(M&, V&, V&) should have exactly the type V");

        V vec1 = leftVector<V>();
        V vec2 = rightVector<V>();

        DUNE_SIMD_CHECK(allTrue(cond(M(true),  vec1, vec2) == vec1));
        DUNE_SIMD_CHECK(allTrue(cond(M(false), vec1, vec2) == vec2));

        V mixedResult;
        M mixedMask;
        for(std::size_t l = 0; l < lanes(mixedMask); ++l)
        {
          lane(l, mixedMask  ) = (l % 2);
          lane(l, mixedResult) = lane(l, (l % 2) ? vec1 : vec2);
        }

        DUNE_SIMD_CHECK(allTrue(cond(mixedMask, vec1, vec2) == mixedResult));
      }

      template<class V>
      std::enable_if_t<!Impl::LessThenComparable<Scalar<V> >::value>
      checkMinMax() {}

      template<class V>
      std::enable_if_t<Impl::LessThenComparable<Scalar<V> >::value>
      checkMinMax()
      {
        static_assert
          (std::is_same<decltype(max(std::declval<V>())), Scalar<V> >::value,
           "The result of max(V) should be exactly Scalar<V>");

        static_assert
          (std::is_same<decltype(min(std::declval<V>())), Scalar<V> >::value,
           "The result of min(V) should be exactly Scalar<V>");

        static_assert
          (std::is_same<decltype(max(std::declval<V&>())), Scalar<V> >::value,
           "The result of max(V) should be exactly Scalar<V>");

        static_assert
          (std::is_same<decltype(min(std::declval<V&>())), Scalar<V> >::value,
           "The result of min(V) should be exactly Scalar<V>");

        const V vec1 = leftVector<V>();

        DUNE_SIMD_CHECK(max(vec1) == Scalar<V>(lanes(vec1)));
        DUNE_SIMD_CHECK(min(vec1) == Scalar<V>(1));
      }

#undef DUNE_SIMD_CHECK

    public:
      //! run unit tests for simd vector type V
      /**
       * This function will also ensure that \c checkVector<Index<V>>() and \c
       * checkMask<Mask<V>>() are run.  No test will be run twice for a given
       * type.
       *
       * \note As an implementor of a unit test, you are encouraged to
       *       explicitly instantiate this function in seperate compilation
       *       units for the types you are testing.  Look at `standardtest.cc`
       *       for how to do this.
       *
       * Background: The compiler can use a lot of memory when compiling a
       * unit test for many Simd vector types.  E.g. for standardtest.cc,
       * which tests all the fundamental arithmetic types plus \c
       * std::complex, g++ 4.9.2 (-g -O0 -Wall on x86_64 GNU/Linux) used
       * ~6GByte.
       *
       * One mitigation is to explicitly instantiate \c checkVector() for the
       * types that are tested.  Still after doing that, standardtest.cc
       * needed ~1.5GByte during compilation, which is more than the
       * compilation units that actually instantiated \c checkVector() (which
       * clocked in at maximum at around 800MB, depending on how many
       * instantiations they contained).
       *
       * The second mitigation is to define \c checkVector() outside of the
       * class.  I have no idea why this helps, but it makes compilation use
       * less than ~100MByte.  (Yes, functions defined inside the class are
       * implicitly \c inline, but the function is a template so it has inline
       * semantics even when defined outside of the class.  And I tried \c
       * __attribute__((__noinline__)), which had no effect on memory
       * consumption.)
       */
      template<class V>
      void checkVector();

      //! run unit tests for simd mask type V
      /**
       * This function will also ensure that \c checkVector<Index<V>>() is
       * run.  No test will be run twice for a given type.
       *
       * \note As an implementor of a unit test, you are encouraged to
       *       explicitly instantiate this function in seperate compilation
       *       units for the types you are testing.  Look at `standardtest.cc`
       *       for how to do this.  See \c checkVector() for background on
       *       this.
       */
      template<class V>
      void checkMask();

      //! run unit tests for simd index type V
      /**
       * This function will also ensure that `Index<V>` is the same type as
       * `V` and run `checkMask<Mask<V>>()`.  No test will be run twice for a
       * given type.
       *
       * \note As an implementor of a unit test, you are encouraged to
       *       explicitly instantiate this function in seperate compilation
       *       units for the types you are testing.  Look at `standardtest.cc`
       *       for how to do this.  See \c checkVector() for background on
       *       this.
       */
      template<class V>
      void checkIndex();

      //! whether all tests succeeded
      bool good() const
      {
        return good_;
      }

    }; // class UnitTest

    // Needs to be defined outside of the class to bring memory consumption
    // during compilation down to an acceptable level.
    template<class V>
    void UnitTest::checkVector()
    {
      // check whether the test for this type already started
      if(seen_.emplace(typeid (V)).second == false)
      {
        // type already seen, nothing to do
        return;
      }

      // do these first so everything that appears after "Checking SIMD type
      // ..." really pertains to that type
      checkIndexOf<V>();
      checkMaskOf<V>();

      log_ << "Checking SIMD vector type " << className<V>() << std::endl;

      checkLanes<V>();
      checkScalar<V>();

      checkDefaultConstruct<V>();
      checkLane<V>();
      checkCopyMoveConstruct<V>();
      checkBroadcastVectorConstruct<V>();
      checkBracedAssign<V>();
      checkBracedBroadcastAssign<V>();

      checkVectorOps<V>();

      checkAutoCopy<V>();
      checkCond<V>();

      // checkBoolReductions<V>(); // not applicable

      checkMinMax<V>();
    }

    template<class I>
    void UnitTest::checkIndex()
    {
      static_assert(std::is_same<I, std::decay_t<I> >::value, "Index types "
                    "must not be references, and must not include "
                    "cv-qualifiers");
      static_assert(std::is_same<I, Index<I> >::value,
                    "Index types must be their own index type");
      static_assert(std::is_integral<Scalar<I> >::value,
                    "Index types scalar must be integral");

      // check whether the test for this type already started
      if(indexSeen_.emplace(typeid (I)).second == false)
      {
        // type already seen, nothing to do
        return;
      }

      // do these first so everything that appears after "Checking SIMD type
      // ..." really pertains to that type
      checkMaskOf<I>(); // not applicable

      log_ << "Checking SIMD index type " << className<I>() << std::endl;

      checkLanes<I>();
      checkScalar<I>();

      checkDefaultConstruct<I>();
      checkLane<I>();
      checkCopyMoveConstruct<I>();
      checkBroadcastVectorConstruct<I>();
      checkBracedAssign<I>();
      checkBracedBroadcastAssign<I>();

      checkIndexOps<I>();

      checkAutoCopy<I>();
      checkCond<I>();

      // checkBoolReductions<V>(); // not applicable

      checkMinMax<I>();
    }

    template<class M>
    void UnitTest::checkMask()
    {
      static_assert(std::is_same<M, std::decay_t<M> >::value, "Mask types "
                    "must not be references, and must not include "
                    "cv-qualifiers");

      static_assert(std::is_same<M, Mask<M> >::value,
                    "Mask must be their own mask types.");

      // check whether the test for this type already started
      if(maskSeen_.emplace(typeid (M)).second == false)
      {
        // type already seen, nothing to do
        return;
      }

      // do these first so everything that appears after "Checking SIMD type
      // ..." really pertains to that type
      checkIndexOf<M>();
      // checkMaskOf<M>(); // not applicable

      log_ << "Checking SIMD mask type " << className<M>() << std::endl;

      checkLanes<M>();
      checkScalar<M>();

      checkDefaultConstruct<M>();
      checkLane<M>();
      checkCopyMoveConstruct<M>();
      checkBroadcastMaskConstruct<M>();
      checkBracedAssign<M>();
      checkBracedBroadcastAssign<M>();

      checkMaskOps<M>();

      checkAutoCopy<M>();
      checkCond<M>();

      checkBoolReductions<M>();

      checkMinMax<M>();
    }

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_HH
