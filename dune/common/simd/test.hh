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

#include <dune/common/classname.hh>
#include <dune/common/simd/simd.hh>
#include <dune/common/unused.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      static bool checkGoodHelper(const char *file, int line, const char *func,
                                  const char *expr, bool result)
      {
        if(!result)
        {
          std::cerr << file << ":" << line << ": In " << func  << ": Error: "
                    << "check (" << expr << ") failed." << std::endl;
        }
        return result;
      }

#define DUNE_CHECK_GOOD(expr)                                           \
      (good &= checkGoodHelper(__FILE__, __LINE__, __func__, #expr, (expr)))

      template<class V>
      bool checkVector();

      template<class V>
      bool checkLanes()
      {
        // check lanes
        static_assert(std::is_same<std::size_t, decltype(lanes<V>())>::value,
                      "return type of lanes<V>() should be std::size_t");
        static_assert(std::is_same<std::size_t, decltype(lanes(V{}))>::value,
                      "return type of lanes(V{}) should be std::size_t");

        // the result of lanes<V>() must be constexpr
        constexpr auto size = lanes<V>();
        // but the result of lanes(vec) does not need to be constexpr
        if(size != lanes(V{}))
        {
          std::cerr << "Error: lanes<V>() and lanes(V{}) differ (" << size
                    << " vs. " << lanes(V{}) << ")" << std::endl;
          return false;
        }
        else
        {
          return true;
        }
      }

      template<class V>
      bool checkScalar()
      {
        // check that the type Scalar<V> exists
        using T = Scalar<V>;

        static_assert(std::is_same<T, std::decay_t<T> >::value, "Scalar types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        T a{};
        //don't complain about unused variables
        (void)a;

        return true;
      }

      template<class V>
      bool checkIndex()
      {
        // check that the type Scalar<V> exists
        using I = Index<V>;
        std::cerr << "Index type of " << className<V>() << " is "
                  << className<I>() << std::endl;


        static_assert(std::is_same<I, std::decay_t<I> >::value, "Index types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        static_assert(lanes<V>() == lanes<I>(), "Index types must have the "
                      "same number of lanes as the original vector types");

        return checkVector<I>();
      }

      template<class V>
      bool checkMask()
      {
        // check that the type Scalar<V> exists
        using M = Mask<V>;
        std::cerr << "Mask type of " << className<V>() << " is "
                  << className<M>() << std::endl;

        static_assert(std::is_same<M, std::decay_t<M> >::value, "Mask types "
                      "must not be references, and must not include "
                      "cv-qualifiers");
        static_assert(lanes<V>() == lanes<M>(), "Mask types must have the "
                      "same number of lanes as the original vector types");

        return checkVector<M>();
      }

      template<class V>
      bool check42(const V &v)
      {
        bool good = true;

        for(std::size_t l = 0; l < lanes(v); ++l)
          // need to cast in case we have a mask type
          DUNE_CHECK_GOOD(lane(l, v) == Scalar<V>(42));

        return good;
      }

      template<class V>
      V make123()
      {
        V vec;
        for(std::size_t l = 0; l < lanes(vec); ++l)
          lane(l, vec) = l + 1;
        return vec;
      }

      template<class V>
      bool check123(const V &v)
      {
        bool good = true;

        for(std::size_t l = 0; l < lanes(v); ++l)
          // need to cast in case we have a mask type
          DUNE_CHECK_GOOD(lane(l, v) == Scalar<V>(l+1));

        return good;
      }

      template<class V>
      bool checkDefaultConstruct()
      {
        bool good = true;

        { V DUNE_UNUSED vec;      }
        { V DUNE_UNUSED vec{};    }
        { V DUNE_UNUSED vec = {}; }

        return good;
      }

      template<class T>
      T rvalue(T t)
      {
        return t;
      }

      template<class> struct show_type;

      template<class V>
      bool checkLane()
      {
        bool good = true;

        V vec;
        // check lane() on mutable lvalues
        for(std::size_t l = 0; l < lanes(vec); ++l)
          lane(l, vec) = l + 1;
        for(std::size_t l = 0; l < lanes(vec); ++l)
          DUNE_CHECK_GOOD(lane(l, vec) == Scalar<V>(l + 1));
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
          DUNE_CHECK_GOOD(lane(l, vec2) == Scalar<V>(l + 1));
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
          DUNE_CHECK_GOOD(lane(l, rvalue(vec)) == Scalar<V>(l + 1));
        using RRes = decltype(lane(0, rvalue(vec)));
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

        return good;
      }

      // check non-default constructors
      template<class V>
      bool checkConstruct()
      {
        bool good = true;

        // elided copy/move constructors
        { V vec(make123<V>());  good &= check123(vec); }
        { V vec{make123<V>()};  good &= check123(vec); }

        // copy constructors
        { V ref = make123<V>(); V vec(ref);
          good &= check123(vec) && check123(ref); }
        { V ref = make123<V>(); V vec{ref};
          good &= check123(vec) && check123(ref); }
        { const V ref = make123<V>(); V vec(ref); good &= check123(vec); }
        { const V ref = make123<V>(); V vec{ref}; good &= check123(vec); }

        // move constructors
        { V ref = make123<V>(); V vec(std::move(ref));
          good &= check123(vec); }
        { V ref = make123<V>(); V vec{std::move(ref)};
          good &= check123(vec); }

        // broadcast copy constructors
        { Scalar<V> ref = 42; V vec(ref);
          good &= check42(vec) && ref == Scalar<V>(42); }
        { Scalar<V> ref = 42; V vec{ref};
          good &= check42(vec) && ref == Scalar<V>(42); }
        { const Scalar<V> ref = 42; V vec(ref); good &= check42(vec); }
        { const Scalar<V> ref = 42; V vec{ref}; good &= check42(vec); }

        // broadcast move constructors
        { Scalar<V> ref = 42; V vec(std::move(ref)); good &= check42(vec); }
        { Scalar<V> ref = 42; V vec{std::move(ref)}; good &= check42(vec); }

        return good;
      }

      template<class V>
      bool checkAssign()
      {
        bool good = true;

        // copy assignment
        { V ref = make123<V>();       V vec; vec = ref;
          good &= check123(vec) && check123(ref); }
        { V ref = make123<V>();       V vec; vec = {ref};
          good &= check123(vec) && check123(ref); }
        { const V ref = make123<V>(); V vec; vec = ref;
          good &= check123(vec); }
        { const V ref = make123<V>(); V vec; vec = {ref};
          good &= check123(vec); }

        // move assignment
        { V vec; vec = make123<V>();   good &= check123(vec); }
        { V vec; vec = {make123<V>()}; good &= check123(vec); }

        // broadcast copy assignment
        { Scalar<V> ref = 42;       V vec; vec = ref;
          good &= check42(vec) && ref == Scalar<V>(42); }
        { Scalar<V> ref = 42;       V vec; vec = {ref};
          good &= check42(vec) && ref == Scalar<V>(42); }
        { const Scalar<V> ref = 42; V vec; vec = ref;   good &= check42(vec); }
        { const Scalar<V> ref = 42; V vec; vec = {ref}; good &= check42(vec); }

        // broadcast move assignment
        { Scalar<V> ref = 42; V vec; vec = std::move(ref);
          good &= check42(vec); }
        { Scalar<V> ref = 42; V vec; vec = {std::move(ref)};
          good &= check42(vec); }

        return good;
      }

      struct OpBinaryData {
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
            lane(l, res) = Scalar<V>(l*l+1);
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
          return T(23);
        }
      };

      struct OpBinaryShift : OpBinaryData
      {
        template<class V>
        V rightVector() const
        {
          V res;
          for(std::size_t l = 0; l < lanes(res); ++l)
            // do not exceed number of bits in char
            lane(l, res) = Scalar<V>((l+1)%8);
          return res;
        }

        template<class T>
        T rightScalar() const
        {
          // do not exceed number of bits in char
          return T(5);
        }
      };

#define DUNE_DEFINE_BINARY_OP(NAME, DATA, SYMBOL)                       \
      struct OpBinary##NAME : OpBinary##DATA                            \
      {                                                                 \
        template<class V1, class V2>                                    \
        auto operator()(V1&& v1, V2&& v2) const                         \
          -> decltype(std::forward<V1>(v1) SYMBOL std::forward<V2>(v2)) \
        {                                                               \
          return std::forward<V1>(v1) SYMBOL std::forward<V2>(v2);      \
        }                                                               \
      }

      DUNE_DEFINE_BINARY_OP(Mul,              Data,  *  );
      DUNE_DEFINE_BINARY_OP(Div,              Data,  /  );
      DUNE_DEFINE_BINARY_OP(Remainder,        Data,  %  );

      DUNE_DEFINE_BINARY_OP(Plus,             Data,  +  );
      DUNE_DEFINE_BINARY_OP(Minus,            Data,  -  );

      DUNE_DEFINE_BINARY_OP(LeftShift,        Shift, << );
      DUNE_DEFINE_BINARY_OP(RightShift,       Shift, >> );

      DUNE_DEFINE_BINARY_OP(Less,             Data,  <  );
      DUNE_DEFINE_BINARY_OP(Greater,          Data,  >  );
      DUNE_DEFINE_BINARY_OP(LessEqual,        Data,  <  );
      DUNE_DEFINE_BINARY_OP(GreaterEqual,     Data,  >  );

      DUNE_DEFINE_BINARY_OP(Equal,            Data,  == );
      DUNE_DEFINE_BINARY_OP(NotEqual,         Data,  != );

      DUNE_DEFINE_BINARY_OP(BitAnd,           Data,  &  );
      DUNE_DEFINE_BINARY_OP(BitXor,           Data,  ^  );
      DUNE_DEFINE_BINARY_OP(BitOr,            Data,  |  );

      DUNE_DEFINE_BINARY_OP(LogicAnd,         Data,  && );
      DUNE_DEFINE_BINARY_OP(LogicOr,          Data,  || );

      DUNE_DEFINE_BINARY_OP(Assign,           Data,  =  );
      DUNE_DEFINE_BINARY_OP(AssignMul,        Data,  *= );
      DUNE_DEFINE_BINARY_OP(AssignDiv,        Data,  /= );
      DUNE_DEFINE_BINARY_OP(AssignRemainder,  Data,  %= );
      DUNE_DEFINE_BINARY_OP(AssignPlus,       Data,  += );
      DUNE_DEFINE_BINARY_OP(AssignMinus,      Data,  -= );
      DUNE_DEFINE_BINARY_OP(AssignLeftShift,  Data,  <<=);
      DUNE_DEFINE_BINARY_OP(AssignRightShift, Data,  >>=);
      DUNE_DEFINE_BINARY_OP(AssignAnd,        Data,  &= );
      DUNE_DEFINE_BINARY_OP(AssignXor,        Data,  ^= );
      DUNE_DEFINE_BINARY_OP(AssignOr,         Data,  |= );

#define DUNE_COMMA_SYMBOL ,
      DUNE_DEFINE_BINARY_OP(Comma,            Data,  DUNE_COMMA_SYMBOL);
#undef DUNE_COMMA_SYMBOL

#undef DUNE_DEFINE_BINARY_OP

      template<class>
      struct VoidType
      {
        using type = void;
      };
      template<class T>
      using Void = typename VoidType<T>::type;
      template<class Expr, class = void>
      struct CanCall;
      template<class Op, class... Args, class Dummy>
      struct CanCall<Op(Args...), Dummy> : std::false_type {};
      template<class Op, class... Args>
      struct CanCall<Op(Args...), Void<std::result_of_t<Op(Args...)> > >
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

      template<class V1, class V2, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V1>())),
                   decltype(lane(0, std::declval<V2>())))>::value,
        bool>
      checkBinaryOpVV(Op op)
      {
        static_assert(std::is_same<std::decay_t<V1>, std::decay_t<V2> >::value,
                      "Internal testsystem error: called with two types that "
                      "don't decay to the same thing");

        bool good = true;

        // arguments
        auto val1 = op.template leftVector<std::decay_t<V1>>();
        auto val2 = op.template rightVector<std::decay_t<V2>>();

        // copy the arguments in case V1 or V2 are references
        auto arg1 = val1;
        auto arg2 = val2;
        auto &&result = op(static_cast<V1>(arg1), static_cast<V2>(arg2));
        for(std::size_t l = 0; l < lanes(val1); ++l)
          DUNE_CHECK_GOOD(lane(l, result) ==
                          op(lane(l, static_cast<V1>(val1)),
                             lane(l, static_cast<V2>(val2))));
        // op might modify val1 and val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_CHECK_GOOD(lane(l, val1) == lane(l, arg1));
          DUNE_CHECK_GOOD(lane(l, val2) == lane(l, arg2));
        }

        return good;
      }

      template<class V1, class V2, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V1>())),
                    decltype(lane(0, std::declval<V2>())))>::value,
        bool>
      checkBinaryOpVV(Op op)
      {
        // std::cerr << "No "
        //           << className<Op(decltype(lane(0, std::declval<V1>())),
        //                           decltype(lane(0, std::declval<V2>())))>()
        //           << std::endl
        //           << " ==> Not checking " << className<Op(V1, V2)>()
        //           << std::endl;
        // there is no op for that scalar so don't check for the vector
        return true;
      }

      template<class V, class Op>
      bool checkBinaryOpVV(Op op)
      {
        bool good = true;

        good &= checkBinaryOpVV<V&, V&>(op);
        good &= checkBinaryOpVV<V&, const V&>(op);
        good &= checkBinaryOpVV<V&, V&&>(op);
        good &= checkBinaryOpVV<V&, const V&&>(op);

        good &= checkBinaryOpVV<const V&, V&>(op);
        good &= checkBinaryOpVV<const V&, const V&>(op);
        good &= checkBinaryOpVV<const V&, V&&>(op);
        good &= checkBinaryOpVV<const V&, const V&&>(op);

        good &= checkBinaryOpVV<V&&, V&>(op);
        good &= checkBinaryOpVV<V&&, const V&>(op);
        good &= checkBinaryOpVV<V&&, V&&>(op);
        good &= checkBinaryOpVV<V&&, const V&&>(op);

        good &= checkBinaryOpVV<const V&&, V&>(op);
        good &= checkBinaryOpVV<const V&&, const V&>(op);
        good &= checkBinaryOpVV<const V&&, V&&>(op);
        good &= checkBinaryOpVV<const V&&, const V&&>(op);

        return good;
      }

      template<class V>
      bool checkBinaryOpsVV()
      {
        bool good = true;

        good &= checkBinaryOpVV<V>(OpBinaryMul{});
        good &= checkBinaryOpVV<V>(OpBinaryDiv{});
        good &= checkBinaryOpVV<V>(OpBinaryRemainder{});

        good &= checkBinaryOpVV<V>(OpBinaryPlus{});
        good &= checkBinaryOpVV<V>(OpBinaryMinus{});

        good &= checkBinaryOpVV<V>(OpBinaryLeftShift{});
        good &= checkBinaryOpVV<V>(OpBinaryRightShift{});

        good &= checkBinaryOpVV<V>(OpBinaryLess{});
        good &= checkBinaryOpVV<V>(OpBinaryGreater{});
        good &= checkBinaryOpVV<V>(OpBinaryLessEqual{});
        good &= checkBinaryOpVV<V>(OpBinaryGreaterEqual{});

        good &= checkBinaryOpVV<V>(OpBinaryEqual{});
        good &= checkBinaryOpVV<V>(OpBinaryNotEqual{});

        good &= checkBinaryOpVV<V>(OpBinaryBitAnd{});
        good &= checkBinaryOpVV<V>(OpBinaryBitXor{});
        good &= checkBinaryOpVV<V>(OpBinaryBitOr{});

        good &= checkBinaryOpVV<V>(OpBinaryLogicAnd{});
        good &= checkBinaryOpVV<V>(OpBinaryLogicOr{});

        good &= checkBinaryOpVV<V>(OpBinaryAssign{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignMul{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignDiv{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignRemainder{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignPlus{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignMinus{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignLeftShift{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignRightShift{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignAnd{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignXor{});
        good &= checkBinaryOpVV<V>(OpBinaryAssignOr{});

        good &= checkBinaryOpVV<V>(OpBinaryComma{});

        return good;
      }

      template<class T1, class V2, class Op>
      std::enable_if_t<
        CanCall<Op(T1, decltype(lane(0, std::declval<V2>())))>::value,
        bool>
      checkBinaryOpSV(Op op)
      {
        static_assert(std::is_same<std::decay_t<T1>,
                      Scalar<std::decay_t<V2> > >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        bool good = true;

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
          DUNE_CHECK_GOOD(lane(l, sresult) ==
                          op(        static_cast<T1>(sval1),
                             lane(l, static_cast<V2>(sval2))));
          DUNE_CHECK_GOOD(lane(l, vresult) ==
                          op(lane(l, static_cast<V1>(vval1)),
                             lane(l, static_cast<V2>(vval2))));
          // cross check
          DUNE_CHECK_GOOD(lane(l, sresult) == lane(l, vresult));
        }
        // op might modify [sv]val1 and [sv]val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_CHECK_GOOD(        sval1  == lane(l, sarg1));
          DUNE_CHECK_GOOD(lane(l, sval2) == lane(l, sarg2));
          DUNE_CHECK_GOOD(lane(l, vval1) == lane(l, varg1));
          DUNE_CHECK_GOOD(lane(l, vval2) == lane(l, varg2));
          // cross check
          DUNE_CHECK_GOOD(        sval1  == lane(l, vval1));
          DUNE_CHECK_GOOD(lane(l, sval2) == lane(l, vval2));
        }

        return good;
      }

      template<class T1, class V2, class Op>
      std::enable_if_t<
        !CanCall<Op(T1, decltype(lane(0, std::declval<V2>())))>::value,
        bool>
      checkBinaryOpSV(Op op)
      {
        // std::cerr << "No "
        //           << className<Op(T1, decltype(lane(0, std::declval<V2>())))>()
        //           << std::endl
        //           << " ==> Not checking " << className<Op(T1, V2)>()
        //           << std::endl;
        // there is no op for that scalar so don't check for the vector
        return true;
      }

      template<class V, class Op>
      bool checkBinaryOpSV(Op op)
      {
        using S = Scalar<V>;

        bool good = true;

        good &= checkBinaryOpSV<S&, V&>(op);
        good &= checkBinaryOpSV<S&, const V&>(op);
        good &= checkBinaryOpSV<S&, V&&>(op);
        good &= checkBinaryOpSV<S&, const V&&>(op);

        good &= checkBinaryOpSV<const S&, V&>(op);
        good &= checkBinaryOpSV<const S&, const V&>(op);
        good &= checkBinaryOpSV<const S&, V&&>(op);
        good &= checkBinaryOpSV<const S&, const V&&>(op);

        good &= checkBinaryOpSV<S&&, V&>(op);
        good &= checkBinaryOpSV<S&&, const V&>(op);
        good &= checkBinaryOpSV<S&&, V&&>(op);
        good &= checkBinaryOpSV<S&&, const V&&>(op);

        good &= checkBinaryOpSV<const S&&, V&>(op);
        good &= checkBinaryOpSV<const S&&, const V&>(op);
        good &= checkBinaryOpSV<const S&&, V&&>(op);
        good &= checkBinaryOpSV<const S&&, const V&&>(op);

        return good;
      }


      template<class V>
      bool checkBinaryOpsSV()
      {
        bool good = true;

        good &= checkBinaryOpSV<V>(OpBinaryMul{});
        good &= checkBinaryOpSV<V>(OpBinaryDiv{});
        good &= checkBinaryOpSV<V>(OpBinaryRemainder{});

        good &= checkBinaryOpSV<V>(OpBinaryPlus{});
        good &= checkBinaryOpSV<V>(OpBinaryMinus{});

        good &= checkBinaryOpSV<V>(OpBinaryLeftShift{});
        good &= checkBinaryOpSV<V>(OpBinaryRightShift{});

        good &= checkBinaryOpSV<V>(OpBinaryLess{});
        good &= checkBinaryOpSV<V>(OpBinaryGreater{});
        good &= checkBinaryOpSV<V>(OpBinaryLessEqual{});
        good &= checkBinaryOpSV<V>(OpBinaryGreaterEqual{});

        good &= checkBinaryOpSV<V>(OpBinaryEqual{});
        good &= checkBinaryOpSV<V>(OpBinaryNotEqual{});

        good &= checkBinaryOpSV<V>(OpBinaryBitAnd{});
        good &= checkBinaryOpSV<V>(OpBinaryBitXor{});
        good &= checkBinaryOpSV<V>(OpBinaryBitOr{});

        good &= checkBinaryOpSV<V>(OpBinaryLogicAnd{});
        good &= checkBinaryOpSV<V>(OpBinaryLogicOr{});

        good &= checkBinaryOpSV<V>(OpBinaryComma{});

        return good;
      }

      template<class V1, class T2, class Op>
      std::enable_if_t<
        CanCall<Op(decltype(lane(0, std::declval<V1>())), T2)>::value,
        bool>
      checkBinaryOpVS(Op op)
      {
        static_assert(std::is_same<Scalar<std::decay_t<V1> >,
                      std::decay_t<T2> >::value,
                      "Internal testsystem error: called with a scalar that "
                      "does not match the vector type.");

        bool good = true;

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
          DUNE_CHECK_GOOD(lane(l, sresult) ==
                          op(lane(l, static_cast<V1>(sval1)),
                                     static_cast<T2>(sval2) ));
          DUNE_CHECK_GOOD(lane(l, vresult) ==
                          op(lane(l, static_cast<V1>(vval1)),
                             lane(l, static_cast<V2>(vval2))));
          // cross check
          DUNE_CHECK_GOOD(lane(l, sresult) == lane(l, vresult));
        }
        // op might modify [sv]val1 and [sv]val2, verify that any such
        // modification also happens in the vector case
        for(std::size_t l = 0; l < lanes<std::decay_t<V1> >(); ++l)
        {
          DUNE_CHECK_GOOD(lane(l, sval1) == lane(l, sarg1));
          DUNE_CHECK_GOOD(        sval2  == lane(l, sarg2));
          DUNE_CHECK_GOOD(lane(l, vval1) == lane(l, varg1));
          DUNE_CHECK_GOOD(lane(l, vval2) == lane(l, varg2));
          // cross check
          DUNE_CHECK_GOOD(        sval1  == lane(l, vval1));
          DUNE_CHECK_GOOD(lane(l, sval2) == lane(l, vval2));
        }

        return good;
      }

      template<class V1, class T2, class Op>
      std::enable_if_t<
        !CanCall<Op(decltype(lane(0, std::declval<V1>())), T2)>::value,
        bool>
      checkBinaryOpVS(Op op)
      {
        // std::cerr << "No "
        //           << className<Op(decltype(lane(0, std::declval<V1>())), T2)>()
        //           << std::endl
        //           << " ==> Not checking " << className<Op(V1, T2)>()
        //           << std::endl;
        // there is no op for that scalar so don't check for the vector
        return true;
      }

      template<class V, class Op>
      bool checkBinaryOpVS(Op op)
      {
        using S = Scalar<V>;

        bool good = true;

        good &= checkBinaryOpVS<V&, S&>(op);
        good &= checkBinaryOpVS<V&, const S&>(op);
        good &= checkBinaryOpVS<V&, S&&>(op);
        good &= checkBinaryOpVS<V&, const S&&>(op);

        good &= checkBinaryOpVS<const V&, S&>(op);
        good &= checkBinaryOpVS<const V&, const S&>(op);
        good &= checkBinaryOpVS<const V&, S&&>(op);
        good &= checkBinaryOpVS<const V&, const S&&>(op);

        good &= checkBinaryOpVS<V&&, S&>(op);
        good &= checkBinaryOpVS<V&&, const S&>(op);
        good &= checkBinaryOpVS<V&&, S&&>(op);
        good &= checkBinaryOpVS<V&&, const S&&>(op);

        good &= checkBinaryOpVS<const V&&, S&>(op);
        good &= checkBinaryOpVS<const V&&, const S&>(op);
        good &= checkBinaryOpVS<const V&&, S&&>(op);
        good &= checkBinaryOpVS<const V&&, const S&&>(op);

        return good;
      }


      template<class V>
      bool checkBinaryOpsVS()
      {
        bool good = true;

        good &= checkBinaryOpVS<V>(OpBinaryMul{});
        good &= checkBinaryOpVS<V>(OpBinaryDiv{});
        good &= checkBinaryOpVS<V>(OpBinaryRemainder{});

        good &= checkBinaryOpVS<V>(OpBinaryPlus{});
        good &= checkBinaryOpVS<V>(OpBinaryMinus{});

        good &= checkBinaryOpVS<V>(OpBinaryLeftShift{});
        good &= checkBinaryOpVS<V>(OpBinaryRightShift{});

        good &= checkBinaryOpVS<V>(OpBinaryLess{});
        good &= checkBinaryOpVS<V>(OpBinaryGreater{});
        good &= checkBinaryOpVS<V>(OpBinaryLessEqual{});
        good &= checkBinaryOpVS<V>(OpBinaryGreaterEqual{});

        good &= checkBinaryOpVS<V>(OpBinaryEqual{});
        good &= checkBinaryOpVS<V>(OpBinaryNotEqual{});

        good &= checkBinaryOpVS<V>(OpBinaryBitAnd{});
        good &= checkBinaryOpVS<V>(OpBinaryBitXor{});
        good &= checkBinaryOpVS<V>(OpBinaryBitOr{});

        good &= checkBinaryOpVS<V>(OpBinaryLogicAnd{});
        good &= checkBinaryOpVS<V>(OpBinaryLogicOr{});

        good &= checkBinaryOpVS<V>(OpBinaryAssign{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignMul{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignDiv{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignRemainder{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignPlus{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignMinus{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignLeftShift{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignRightShift{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignAnd{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignXor{});
        good &= checkBinaryOpVS<V>(OpBinaryAssignOr{});

        good &= checkBinaryOpVS<V>(OpBinaryComma{});

        return good;
      }

      template<class V>
      bool checkVector()
      {
        bool first = false;
        // We're not actually interested in first_anchor.  But its initializer
        // is executed only once, meaning first will be true only in one
        // invocation of checkVector<V>() (for a given V)
        static const bool DUNE_UNUSED first_anchor = (first = true);
        if(!first) return true;

        // accumulate status of runtime tests
        bool good = true;

        // do these first so everything that appears after "Checking SIMD type
        // ..." really pertains to that type
        good &= checkIndex<V>();
        good &= checkMask<V>();

        std::cerr << "Checking SIMD type " << className<V>() << std::endl;

        good &= checkLanes<V>();
        good &= checkScalar<V>();

        good &= checkDefaultConstruct<V>();
        good &= checkLane<V>();
        good &= checkConstruct<V>();
        good &= checkAssign<V>();

        good &= checkBinaryOpsVV<V>();
        good &= checkBinaryOpsSV<V>();
        good &= checkBinaryOpsVS<V>();

        return good;
      }

#undef DUNE_CHECK_GOOD

    } // namespace Test
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_HH
