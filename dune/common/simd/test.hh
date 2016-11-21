#ifndef DUNE_COMMON_SIMD_TEST_HH
#define DUNE_COMMON_SIMD_TEST_HH

/** @file
 *  @brief Common tests for simd abstraction implementations
 *
 * This file is an interface header and may be included without restrictions.
 */

#include <cstddef>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>

#include <dune/common/classname.hh>
#include <dune/common/simd/simd.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/unused.hh>

namespace Dune {
  namespace Simd {

    namespace Impl {

      template<class Expr, bool = false>
      struct CanCall;
      template<class Op, class... Args, bool dummy>
      struct CanCall<Op(Args...), dummy> : std::false_type {};
      template<class Op, class... Args>
      struct CanCall<Op(Args...),
                     AlwaysFalse<std::result_of_t<Op(Args...)> >::value>
        : std::true_type
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

      ////////////////////////////////////////////////////////////////////////
      //
      //  Helper functions
      //

      void complain(const char *file, int line, const char *func,
                    const char *expr);

      // This macro is defined only within this file, do not use anywhere
      // else.  Doing the actual printing in an external function dramatically
      // reduces memory use during compilation.  Defined in such a way that
      // the call will only happen for failed checks.
#define DUNE_SIMD_CHECK(expr)                                           \
      ((expr) ? void() : complain(__FILE__, __LINE__, __func__, #expr))

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
      void checkIndex()
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

        checkSimdType<I>();
      }

      template<class V>
      void checkMask()
      {
        // check that the type Scalar<V> exists
        using M = Mask<V>;
        log_ << "Mask type of " << className<V>() << " is " << className<M>()
             << std::endl;

        static_assert(std::is_same<M, std::decay_t<M> >::value, "Mask types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        static_assert(lanes<V>() == lanes<M>(), "Mask types must have the "
                      "same number of lanes as the original vector types");

        checkSimdType<M>();
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
      void checkConstruct()
      {
        // elided copy/move constructors
        { V vec(make123<V>()); DUNE_SIMD_CHECK(is123(vec)); }
        { V vec{make123<V>()}; DUNE_SIMD_CHECK(is123(vec)); }

        // copy constructors
        { V ref = make123<V>();       V vec(ref);
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        { V ref = make123<V>();       V vec{ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        { const V ref = make123<V>(); V vec(ref);
          DUNE_SIMD_CHECK(is123(vec)); }
        { const V ref = make123<V>(); V vec{ref};
          DUNE_SIMD_CHECK(is123(vec)); }

        // move constructors
        { V ref = make123<V>(); V vec(std::move(ref));
          DUNE_SIMD_CHECK(is123(vec)); }
        { V ref = make123<V>(); V vec{std::move(ref)};
          DUNE_SIMD_CHECK(is123(vec)); }

        // broadcast copy constructors
        { Scalar<V> ref = 42; V vec(ref);
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        { Scalar<V> ref = 42; V vec{ref};
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        { const Scalar<V> ref = 42; V vec(ref);
          DUNE_SIMD_CHECK(is42(vec)); }
        { const Scalar<V> ref = 42; V vec{ref};
          DUNE_SIMD_CHECK(is42(vec)); }

        // broadcast move constructors
        { Scalar<V> ref = 42; V vec(std::move(ref));
          DUNE_SIMD_CHECK(is42(vec)); }
        { Scalar<V> ref = 42; V vec{std::move(ref)};
          DUNE_SIMD_CHECK(is42(vec)); }
      }

      // assignment is checked in the binary ops as well, but this checks
      // assignment from a braced-init-list
      template<class V>
      void checkAssign()
      {
        // copy assignment
        { V ref = make123<V>();       V vec; vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }
        { const V ref = make123<V>(); V vec; vec = {ref};
          DUNE_SIMD_CHECK(is123(vec)); DUNE_SIMD_CHECK(is123(ref)); }

        // move assignment
        { V vec; vec = {make123<V>()}; DUNE_SIMD_CHECK(is123(vec)); }

        // broadcast copy assignment
        { Scalar<V> ref = 42;       V vec; vec = {ref};
          DUNE_SIMD_CHECK(is42(vec)); DUNE_SIMD_CHECK(ref == Scalar<V>(42)); }
        { const Scalar<V> ref = 42; V vec; vec = {ref};
          DUNE_SIMD_CHECK(is42(vec)); }

        // broadcast move assignment
        { Scalar<V> ref = 42; V vec; vec = {std::move(ref)};
          DUNE_SIMD_CHECK(is42(vec)); }
      }

      // base class for operator data
      struct OpData {
        template<class V>
        V leftVector() const
        {
          V res;
          for(std::size_t l = 0; l < lanes(res); ++l)
            lane(l, res) = Scalar<V>(l+1);
          return res;
        }

        template<class V>
        V rightVector() const
        {
          V res;
          for(std::size_t l = 0; l < lanes(res); ++l)
            // do not exceed number of bits in char (for shifts)
            lane(l, res) = Scalar<V>((l+1)%8);
          return res;
        }

        template<class T>
        T leftScalar() const
        {
          return T(42);
        }

        template<class T>
        T rightScalar() const
        {
          // do not exceed number of bits in char
          return T(5);
        }
      };

      //////////////////////////////////////////////////////////////////////
      //
      // checks for postfix operators
      //

#define DUNE_SIMD_POSTFIX_OP(NAME, SYMBOL)              \
      struct OpPostfix##NAME : OpData                   \
      {                                                 \
        template<class V>                               \
        auto operator()(V&& v) const                    \
          -> decltype(std::forward<V>(v) SYMBOL)        \
        {                                               \
          return std::forward<V>(v) SYMBOL;             \
        }                                               \
      }

      DUNE_SIMD_POSTFIX_OP(Decrement,        -- );
      DUNE_SIMD_POSTFIX_OP(Increment,        ++ );

#undef DUNE_SIMD_POSTFIX_OP

      template<class V, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkPostfixOpV(Op op)
      {
        // arguments
        auto val = op.template leftVector<std::decay_t<V>>();

        // copy the arguments in case V is a references
        auto arg = val;
        auto &&result = op(static_cast<V>(arg));
        for(std::size_t l = 0; l < lanes(val); ++l)
          DUNE_SIMD_CHECK(lane(l, result) == op(lane(l, static_cast<V>(val))));
        // op might modify val, verify that any such modification also happens
        // in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V> >(); ++l)
          DUNE_SIMD_CHECK(lane(l, val) == lane(l, arg));
      }

      template<class V, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkPostfixOpV(Op op)
      {
        // log_ << "No " << className<Op(decltype(lane(0, std::declval<V>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V)>() << std::endl;
      }

      template<class V, class Op>
      void checkPostfixOpsV(Op op)
      {
        checkPostfixOpV<V&>(op);
        checkPostfixOpV<const V&>(op);
        checkPostfixOpV<V&&>(op);
        checkPostfixOpV<const V&&>(op);
      }

      template<class V>
      void checkPostfixOpsV()
      {
        checkPostfixOpsV<V>(OpPostfixDecrement{});
        checkPostfixOpsV<V>(OpPostfixIncrement{});
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for unary (prefix) operators
      //

#define DUNE_SIMD_PREFIX_OP(NAME, SYMBOL)               \
      struct OpPrefix##NAME : OpData                    \
      {                                                 \
        template<class V>                               \
        auto operator()(V&& v) const                    \
          -> decltype(SYMBOL std::forward<V>(v))        \
        {                                               \
          return SYMBOL std::forward<V>(v);             \
        }                                               \
      }

      DUNE_SIMD_PREFIX_OP(Decrement,        -- );
      DUNE_SIMD_PREFIX_OP(Increment,        ++ );

      DUNE_SIMD_PREFIX_OP(Plus,             +  );
      DUNE_SIMD_PREFIX_OP(Minus,            -  );
      DUNE_SIMD_PREFIX_OP(LogicNot,         !  );
      DUNE_SIMD_PREFIX_OP(BitNot,           ~  );

#undef DUNE_SIMD_PREFIX_OP

      template<class V, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkPrefixOpV(Op op)
      {
        // arguments
        auto val = op.template rightVector<std::decay_t<V>>();

        // copy the arguments in case V is a references
        auto arg = val;
        auto &&result = op(static_cast<V>(arg));
        for(std::size_t l = 0; l < lanes(val); ++l)
          DUNE_SIMD_CHECK(lane(l, result) == op(lane(l, static_cast<V>(val))));
        // op might modify val, verify that any such modification also happens
        // in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V> >(); ++l)
          DUNE_SIMD_CHECK(lane(l, val) == lane(l, arg));
      }

      template<class V, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V>())))>::value>
      checkPrefixOpV(Op op)
      {
        // log_ << "No " << className<Op(decltype(lane(0, std::declval<V>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V)>() << std::endl;
      }

      template<class V, class Op>
      void checkPrefixOpsV(Op op)
      {
        checkPrefixOpV<V&>(op);
        checkPrefixOpV<const V&>(op);
        checkPrefixOpV<V&&>(op);
        checkPrefixOpV<const V&&>(op);
      }

      template<class V>
      void checkPrefixOpsV()
      {
        checkPrefixOpsV<V>(OpPrefixDecrement{});
        checkPrefixOpsV<V>(OpPrefixIncrement{});
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for binary operators
      //

#define DUNE_SIMD_BINARY_OP(NAME, SYMBOL)                               \
      struct OpBinary##NAME : OpData                                    \
      {                                                                 \
        template<class V1, class V2>                                    \
        auto operator()(V1&& v1, V2&& v2) const                         \
          -> decltype(std::forward<V1>(v1) SYMBOL std::forward<V2>(v2)) \
        {                                                               \
          return std::forward<V1>(v1) SYMBOL std::forward<V2>(v2);      \
        }                                                               \
      }

      DUNE_SIMD_BINARY_OP(Mul,              *  );
      DUNE_SIMD_BINARY_OP(Div,              /  );
      DUNE_SIMD_BINARY_OP(Remainder,        %  );

      DUNE_SIMD_BINARY_OP(Plus,             +  );
      DUNE_SIMD_BINARY_OP(Minus,            -  );

      DUNE_SIMD_BINARY_OP(LeftShift,        << );
      DUNE_SIMD_BINARY_OP(RightShift,       >> );

      DUNE_SIMD_BINARY_OP(Less,             <  );
      DUNE_SIMD_BINARY_OP(Greater,          >  );
      DUNE_SIMD_BINARY_OP(LessEqual,        <  );
      DUNE_SIMD_BINARY_OP(GreaterEqual,     >  );

      DUNE_SIMD_BINARY_OP(Equal,            == );
      DUNE_SIMD_BINARY_OP(NotEqual,         != );

      DUNE_SIMD_BINARY_OP(BitAnd,           &  );
      DUNE_SIMD_BINARY_OP(BitXor,           ^  );
      DUNE_SIMD_BINARY_OP(BitOr,            |  );

      DUNE_SIMD_BINARY_OP(LogicAnd,         && );
      DUNE_SIMD_BINARY_OP(LogicOr,          || );

      DUNE_SIMD_BINARY_OP(Assign,           =  );
      DUNE_SIMD_BINARY_OP(AssignMul,        *= );
      DUNE_SIMD_BINARY_OP(AssignDiv,        /= );
      DUNE_SIMD_BINARY_OP(AssignRemainder,  %= );
      DUNE_SIMD_BINARY_OP(AssignPlus,       += );
      DUNE_SIMD_BINARY_OP(AssignMinus,      -= );
      DUNE_SIMD_BINARY_OP(AssignLeftShift,  <<=);
      DUNE_SIMD_BINARY_OP(AssignRightShift, >>=);
      DUNE_SIMD_BINARY_OP(AssignAnd,        &= );
      DUNE_SIMD_BINARY_OP(AssignXor,        ^= );
      DUNE_SIMD_BINARY_OP(AssignOr,         |= );

#define DUNE_SIMD_COMMA_SYMBOL ,
      DUNE_SIMD_BINARY_OP(Comma,            DUNE_SIMD_COMMA_SYMBOL);
#undef DUNE_SIMD_COMMA_SYMBOL

#undef DUNE_SIMD_BINARY_OP

      //////////////////////////////////////////////////////////////////////
      //
      // checks for vector-vector binary operations
      //

      template<class V1, class V2, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V1>())),
                   decltype(lane(0, std::declval<V2>())))>::value>
      checkBinaryOpVV(Op op)
      {
        static_assert(std::is_same<std::decay_t<V1>, std::decay_t<V2> >::value,
                      "Internal testsystem error: called with two types that "
                      "don't decay to the same thing");

        // arguments
        auto val1 = op.template leftVector<std::decay_t<V1>>();
        auto val2 = op.template rightVector<std::decay_t<V2>>();

        // copy the arguments in case V1 or V2 are references
        auto arg1 = val1;
        auto arg2 = val2;
        auto &&result = op(static_cast<V1>(arg1), static_cast<V2>(arg2));
        for(std::size_t l = 0; l < lanes(val1); ++l)
          DUNE_SIMD_CHECK(lane(l, result) ==
                          op(lane(l, static_cast<V1>(val1)),
                             lane(l, static_cast<V2>(val2))));
        // op might modify val1 and val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_SIMD_CHECK(lane(l, val1) == lane(l, arg1));
          DUNE_SIMD_CHECK(lane(l, val2) == lane(l, arg2));
        }
      }

      template<class V1, class V2, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V1>())),
                    decltype(lane(0, std::declval<V2>())))>::value>
      checkBinaryOpVV(Op op)
      {
        // log_ << "No " << className<Op(decltype(lane(0, std::declval<V1>())),
        //                               decltype(lane(0, std::declval<V2>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V1, V2)>() << std::endl;
      }

      template<class V, class Op>
      void checkBinaryOpsVV(Op op)
      {
        checkBinaryOpVV<V&, V&>(op);
        checkBinaryOpVV<V&, const V&>(op);
        checkBinaryOpVV<V&, V&&>(op);
        checkBinaryOpVV<V&, const V&&>(op);

        checkBinaryOpVV<const V&, V&>(op);
        checkBinaryOpVV<const V&, const V&>(op);
        checkBinaryOpVV<const V&, V&&>(op);
        checkBinaryOpVV<const V&, const V&&>(op);

        checkBinaryOpVV<V&&, V&>(op);
        checkBinaryOpVV<V&&, const V&>(op);
        checkBinaryOpVV<V&&, V&&>(op);
        checkBinaryOpVV<V&&, const V&&>(op);

        checkBinaryOpVV<const V&&, V&>(op);
        checkBinaryOpVV<const V&&, const V&>(op);
        checkBinaryOpVV<const V&&, V&&>(op);
        checkBinaryOpVV<const V&&, const V&&>(op);
      }

      template<class V>
      void checkBinaryOpsVV()
      {
        checkBinaryOpsVV<V>(OpBinaryMul{});
        checkBinaryOpsVV<V>(OpBinaryDiv{});
        checkBinaryOpsVV<V>(OpBinaryRemainder{});

        checkBinaryOpsVV<V>(OpBinaryPlus{});
        checkBinaryOpsVV<V>(OpBinaryMinus{});

        checkBinaryOpsVV<V>(OpBinaryLeftShift{});
        checkBinaryOpsVV<V>(OpBinaryRightShift{});

        checkBinaryOpsVV<V>(OpBinaryLess{});
        checkBinaryOpsVV<V>(OpBinaryGreater{});
        checkBinaryOpsVV<V>(OpBinaryLessEqual{});
        checkBinaryOpsVV<V>(OpBinaryGreaterEqual{});

        checkBinaryOpsVV<V>(OpBinaryEqual{});
        checkBinaryOpsVV<V>(OpBinaryNotEqual{});

        checkBinaryOpsVV<V>(OpBinaryBitAnd{});
        checkBinaryOpsVV<V>(OpBinaryBitXor{});
        checkBinaryOpsVV<V>(OpBinaryBitOr{});

        checkBinaryOpsVV<V>(OpBinaryLogicAnd{});
        checkBinaryOpsVV<V>(OpBinaryLogicOr{});

        checkBinaryOpsVV<V>(OpBinaryAssign{});
        checkBinaryOpsVV<V>(OpBinaryAssignMul{});
        checkBinaryOpsVV<V>(OpBinaryAssignDiv{});
        checkBinaryOpsVV<V>(OpBinaryAssignRemainder{});
        checkBinaryOpsVV<V>(OpBinaryAssignPlus{});
        checkBinaryOpsVV<V>(OpBinaryAssignMinus{});
        checkBinaryOpsVV<V>(OpBinaryAssignLeftShift{});
        checkBinaryOpsVV<V>(OpBinaryAssignRightShift{});
        checkBinaryOpsVV<V>(OpBinaryAssignAnd{});
        checkBinaryOpsVV<V>(OpBinaryAssignXor{});
        checkBinaryOpsVV<V>(OpBinaryAssignOr{});

        checkBinaryOpsVV<V>(OpBinaryComma{});
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for scalar-vector binary operations
      //

      template<class T1, class V2, class Op>
      std::enable_if_t<
        CanCall<Op(T1, decltype(lane(0, std::declval<V2>())))>::value>
      checkBinaryOpSV(Op op)
      {
        static_assert(std::is_same<std::decay_t<T1>,
                      Scalar<std::decay_t<V2> > >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        using V1 = CopyRefQual<V2, T1>;

        // arguments
        auto sval1 = op.template leftScalar<std::decay_t<T1>>();
        auto sval2 = op.template rightVector<std::decay_t<V2>>();
        // these are used to cross check with pure vector ops
        std::decay_t<V1> vval1 = sval1;
        auto vval2 = sval2;

        // copy the arguments in case V1 or V2 are references
        auto sarg1 = sval1;
        auto sarg2 = sval2;
        auto varg1 = vval1;
        auto varg2 = vval2;

        auto &&sresult = op(static_cast<T1>(sarg1), static_cast<V2>(sarg2));
        auto &&vresult = op(static_cast<V1>(varg1), static_cast<V2>(varg2));
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_SIMD_CHECK(lane(l, sresult) ==
                          op(        static_cast<T1>(sval1),
                             lane(l, static_cast<V2>(sval2))));
          DUNE_SIMD_CHECK(lane(l, vresult) ==
                          op(lane(l, static_cast<V1>(vval1)),
                             lane(l, static_cast<V2>(vval2))));
          // cross check
          DUNE_SIMD_CHECK(lane(l, sresult) == lane(l, vresult));
        }
        // op might modify [sv]val1 and [sv]val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_SIMD_CHECK(        sval1  == lane(l, sarg1));
          DUNE_SIMD_CHECK(lane(l, sval2) == lane(l, sarg2));
          DUNE_SIMD_CHECK(lane(l, vval1) == lane(l, varg1));
          DUNE_SIMD_CHECK(lane(l, vval2) == lane(l, varg2));
          // cross check
          DUNE_SIMD_CHECK(        sval1  == lane(l, vval1));
          DUNE_SIMD_CHECK(lane(l, sval2) == lane(l, vval2));
        }
      }

      template<class T1, class V2, class Op>
      std::enable_if_t<
        !CanCall<Op(T1, decltype(lane(0, std::declval<V2>())))>::value>
      checkBinaryOpSV(Op op)
      {
        // log_ << "No "
        //      << className<Op(T1, decltype(lane(0, std::declval<V2>())))>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(T1, V2)>() << std::endl;
      }

      template<class V, class Op>
      void checkBinaryOpsSV(Op op)
      {
        using S = Scalar<V>;

        checkBinaryOpSV<S&, V&>(op);
        checkBinaryOpSV<S&, const V&>(op);
        checkBinaryOpSV<S&, V&&>(op);
        checkBinaryOpSV<S&, const V&&>(op);

        checkBinaryOpSV<const S&, V&>(op);
        checkBinaryOpSV<const S&, const V&>(op);
        checkBinaryOpSV<const S&, V&&>(op);
        checkBinaryOpSV<const S&, const V&&>(op);

        checkBinaryOpSV<S&&, V&>(op);
        checkBinaryOpSV<S&&, const V&>(op);
        checkBinaryOpSV<S&&, V&&>(op);
        checkBinaryOpSV<S&&, const V&&>(op);

        checkBinaryOpSV<const S&&, V&>(op);
        checkBinaryOpSV<const S&&, const V&>(op);
        checkBinaryOpSV<const S&&, V&&>(op);
        checkBinaryOpSV<const S&&, const V&&>(op);
      }

      template<class V>
      void checkBinaryOpsSV()
      {
        checkBinaryOpsSV<V>(OpBinaryMul{});
        checkBinaryOpsSV<V>(OpBinaryDiv{});
        checkBinaryOpsSV<V>(OpBinaryRemainder{});

        checkBinaryOpsSV<V>(OpBinaryPlus{});
        checkBinaryOpsSV<V>(OpBinaryMinus{});

        checkBinaryOpsSV<V>(OpBinaryLeftShift{});
        checkBinaryOpsSV<V>(OpBinaryRightShift{});

        checkBinaryOpsSV<V>(OpBinaryLess{});
        checkBinaryOpsSV<V>(OpBinaryGreater{});
        checkBinaryOpsSV<V>(OpBinaryLessEqual{});
        checkBinaryOpsSV<V>(OpBinaryGreaterEqual{});

        checkBinaryOpsSV<V>(OpBinaryEqual{});
        checkBinaryOpsSV<V>(OpBinaryNotEqual{});

        checkBinaryOpsSV<V>(OpBinaryBitAnd{});
        checkBinaryOpsSV<V>(OpBinaryBitXor{});
        checkBinaryOpsSV<V>(OpBinaryBitOr{});

        checkBinaryOpsSV<V>(OpBinaryLogicAnd{});
        checkBinaryOpsSV<V>(OpBinaryLogicOr{});

        checkBinaryOpsSV<V>(OpBinaryComma{});
      }

      //////////////////////////////////////////////////////////////////////
      //
      // checks for vector-scalar binary operations
      //

      template<class V1, class T2, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V1>())), T2)>::value>
      checkBinaryOpVS(Op op)
      {
        static_assert(std::is_same<Scalar<std::decay_t<V1> >,
                      std::decay_t<T2> >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        using V2 = CopyRefQual<V1, T2>;

        // arguments
        auto sval1 = op.template leftVector<std::decay_t<V1>>();
        auto sval2 = op.template rightScalar<std::decay_t<T2>>();
        // these are used to cross check with pure vector ops
        auto vval1 = sval1;
        std::decay_t<V2> vval2 = sval2;

        // copy the arguments in case V1 or V2 are references
        auto sarg1 = sval1;
        auto sarg2 = sval2;
        auto varg1 = vval1;
        auto varg2 = vval2;

        auto &&sresult = op(static_cast<V1>(sarg1), static_cast<T2>(sarg2));
        auto &&vresult = op(static_cast<V1>(varg1), static_cast<V2>(varg2));
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_SIMD_CHECK(lane(l, sresult) ==
                          op(lane(l, static_cast<V1>(sval1)),
                                     static_cast<T2>(sval2) ));
          DUNE_SIMD_CHECK(lane(l, vresult) ==
                          op(lane(l, static_cast<V1>(vval1)),
                             lane(l, static_cast<V2>(vval2))));
          // cross check
          DUNE_SIMD_CHECK(lane(l, sresult) == lane(l, vresult));
        }
        // op might modify [sv]val1 and [sv]val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_SIMD_CHECK(lane(l, sval1) == lane(l, sarg1));
          DUNE_SIMD_CHECK(        sval2  == lane(l, sarg2));
          DUNE_SIMD_CHECK(lane(l, vval1) == lane(l, varg1));
          DUNE_SIMD_CHECK(lane(l, vval2) == lane(l, varg2));
          // cross check
          DUNE_SIMD_CHECK(        sval1  == lane(l, vval1));
          DUNE_SIMD_CHECK(lane(l, sval2) == lane(l, vval2));
        }
      }

      template<class V1, class T2, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V1>())), T2)>::value>
      checkBinaryOpVS(Op op)
      {
        // log_ << "No "
        //      << className<Op(decltype(lane(0, std::declval<V1>())), T2)>()
        //      << std::endl
        //      << " ==> Not checking " << className<Op(V1, T2)>() << std::endl;
      }

      template<class V, class Op>
      void checkBinaryOpsVS(Op op)
      {
        using S = Scalar<V>;

        checkBinaryOpVS<V&, S&>(op);
        checkBinaryOpVS<V&, const S&>(op);
        checkBinaryOpVS<V&, S&&>(op);
        checkBinaryOpVS<V&, const S&&>(op);

        checkBinaryOpVS<const V&, S&>(op);
        checkBinaryOpVS<const V&, const S&>(op);
        checkBinaryOpVS<const V&, S&&>(op);
        checkBinaryOpVS<const V&, const S&&>(op);

        checkBinaryOpVS<V&&, S&>(op);
        checkBinaryOpVS<V&&, const S&>(op);
        checkBinaryOpVS<V&&, S&&>(op);
        checkBinaryOpVS<V&&, const S&&>(op);

        checkBinaryOpVS<const V&&, S&>(op);
        checkBinaryOpVS<const V&&, const S&>(op);
        checkBinaryOpVS<const V&&, S&&>(op);
        checkBinaryOpVS<const V&&, const S&&>(op);
      }

      template<class V>
      void checkBinaryOpsVS()
      {
        checkBinaryOpsVS<V>(OpBinaryMul{});
        checkBinaryOpsVS<V>(OpBinaryDiv{});
        checkBinaryOpsVS<V>(OpBinaryRemainder{});

        checkBinaryOpsVS<V>(OpBinaryPlus{});
        checkBinaryOpsVS<V>(OpBinaryMinus{});

        checkBinaryOpsVS<V>(OpBinaryLeftShift{});
        checkBinaryOpsVS<V>(OpBinaryRightShift{});

        checkBinaryOpsVS<V>(OpBinaryLess{});
        checkBinaryOpsVS<V>(OpBinaryGreater{});
        checkBinaryOpsVS<V>(OpBinaryLessEqual{});
        checkBinaryOpsVS<V>(OpBinaryGreaterEqual{});

        checkBinaryOpsVS<V>(OpBinaryEqual{});
        checkBinaryOpsVS<V>(OpBinaryNotEqual{});

        checkBinaryOpsVS<V>(OpBinaryBitAnd{});
        checkBinaryOpsVS<V>(OpBinaryBitXor{});
        checkBinaryOpsVS<V>(OpBinaryBitOr{});

        checkBinaryOpsVS<V>(OpBinaryLogicAnd{});
        checkBinaryOpsVS<V>(OpBinaryLogicOr{});

        checkBinaryOpsVS<V>(OpBinaryAssign{});
        checkBinaryOpsVS<V>(OpBinaryAssignMul{});
        checkBinaryOpsVS<V>(OpBinaryAssignDiv{});
        checkBinaryOpsVS<V>(OpBinaryAssignRemainder{});
        checkBinaryOpsVS<V>(OpBinaryAssignPlus{});
        checkBinaryOpsVS<V>(OpBinaryAssignMinus{});
        checkBinaryOpsVS<V>(OpBinaryAssignLeftShift{});
        checkBinaryOpsVS<V>(OpBinaryAssignRightShift{});
        checkBinaryOpsVS<V>(OpBinaryAssignAnd{});
        checkBinaryOpsVS<V>(OpBinaryAssignXor{});
        checkBinaryOpsVS<V>(OpBinaryAssignOr{});

        checkBinaryOpsVS<V>(OpBinaryComma{});
      }

#undef DUNE_SIMD_CHECK

    public:
      //! run unit tests for simd type V
      /**
       * This function will also ensure that \c checkSimdType<Index<V>>() and
       * \c checkSimdType<Mask<V>>() are run.  No test will be run twice for a
       * given type.
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
       * One mitigation is to explicitly instantiate \c checkSimdType() for
       * the types that are tested.  Still after doing that, standardtest.cc
       * needed ~1.5GByte during compilation, which is more than the
       * compilation units that actually instantiated \c checkSimdType()
       * (which clocked in at maximum at around 800MB, depending on how many
       * instantiations they contained).
       *
       * The second mitigation is to define \c checkSimdType() outside of the
       * class.  I have no idea why this helps, but it makes compilation use
       * less than ~100MByte.  (Yes, functions defined inside the class are
       * implicitly \c inline, but the function is a template so it has inline
       * semantics even when defined outside of the class.  And I tried \c
       * __attribute__((__noinline__)), which had no effect on memory
       * consumption.)
       */
      template<class V>
      void checkSimdType();

      //! whether all tests succeeded
      bool good() const
      {
        return good_;
      }

    }; // class UnitTest

    // Needs to be defined outside of the class to bring memory consumption
    // during compilation down to an acceptable level.
    template<class V>
    void UnitTest::checkSimdType()
    {
      // check whether the test for this type already started
      if(seen_.emplace(typeid (V)).second == false)
      {
        // type already seen, nothing to do
        return;
      }

      // do these first so everything that appears after "Checking SIMD type
      // ..." really pertains to that type
      checkIndex<V>();
      checkMask<V>();

      log_ << "Checking SIMD type " << className<V>() << std::endl;

      checkLanes<V>();
      checkScalar<V>();

      checkDefaultConstruct<V>();
      checkLane<V>();
      checkConstruct<V>();
      checkAssign<V>();

      checkPostfixOpsV<V>();
      checkPrefixOpsV<V>();

      checkBinaryOpsVV<V>();
      checkBinaryOpsSV<V>();
      checkBinaryOpsVS<V>();
    }

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_HH
