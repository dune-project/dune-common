// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "float_cmp.hh"

#include <vector>
#include <limits>
#include <algorithm>
#include <cstdlib>
#include <dune/common/fvector.hh>

namespace Dune {


  namespace FloatCmp {
    // traits
    //! Mapping of value type to epsilon type
    /**
     * @ingroup FloatCmp
     * @tparam T The value type
     */
    template<class T> struct EpsilonType {
      //! The epsilon type corresponding to value type T
      typedef T Type;
    };
    //! Specialization of EpsilonType for std::vector
    /**
     * @ingroup FloatCmp
     * @tparam T The value_type of the std::vector
     * @tparam A The Allocator of the std::vector
     */
    template<class T, typename A>
    struct EpsilonType<std::vector<T, A> > {
      //! The epsilon type corresponding to value type std::vector<T, A>
      typedef typename EpsilonType<T>::Type Type;
    };
    //! Specialization of EpsilonType for Dune::FieldVector
    /**
     * @ingroup FloatCmp
     * @tparam T The field_type of the Dune::FieldVector
     * @tparam n The size of the Dune::FieldVector
     */
    template<class T, int n>
    struct EpsilonType<FieldVector<T, n> > {
      //! The epsilon type corresponding to value type Dune::FieldVector<T, n>
      typedef typename EpsilonType<T>::Type Type;
    };

    // default epsilon
    template<class T>
    struct DefaultEpsilon<T, relativeWeak> {
      static typename EpsilonType<T>::Type value()
      { return std::numeric_limits<typename EpsilonType<T>::Type>::epsilon()*8.; }
    };
    template<class T>
    struct DefaultEpsilon<T, relativeStrong> {
      static typename EpsilonType<T>::Type value()
      { return std::numeric_limits<typename EpsilonType<T>::Type>::epsilon()*8.; }
    };
    template<class T>
    struct DefaultEpsilon<T, absolute> {
      static typename EpsilonType<T>::Type value()
      { return std::max(std::numeric_limits<typename EpsilonType<T>::Type>::epsilon(), 1e-6); }
    };

    namespace Impl {
      // basic comparison
      template<class T, CmpStyle style = defaultCmpStyle>
      struct eq_t;
      template<class T>
      struct eq_t<T, relativeWeak> {
        static bool eq(const T &first,
                       const T &second,
                       typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T>::value())
        {
          using std::abs;
          return abs(first - second) <= epsilon*std::max(abs(first), abs(second));
        }
      };
      template<class T>
      struct eq_t<T, relativeStrong> {
        static bool eq(const T &first,
                       const T &second,
                       typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T>::value())
        {
          using std::abs;
          return abs(first - second) <= epsilon*std::min(abs(first), abs(second));
        }
      };
      template<class T>
      struct eq_t<T, absolute> {
        static bool eq(const T &first,
                       const T &second,
                       typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T>::value())
        {
          using std::abs;
          return abs(first-second) <= epsilon;
        }
      };
      template<class T, CmpStyle cstyle>
      struct eq_t_std_vec {
        typedef std::vector<T> V;
        static bool eq(const V &first,
                       const V &second,
                       typename EpsilonType<V>::Type epsilon = DefaultEpsilon<V>::value()) {
          auto size = first.size();
          if(size != second.size()) return false;
          for(unsigned int i = 0; i < size; ++i)
            if(!eq_t<T, cstyle>::eq(first[i], second[i], epsilon))
              return false;
          return true;
        }
      };
      template< class T>
      struct eq_t<std::vector<T>, relativeWeak> : eq_t_std_vec<T, relativeWeak> {};
      template< class T>
      struct eq_t<std::vector<T>, relativeStrong> : eq_t_std_vec<T, relativeStrong> {};
      template< class T>
      struct eq_t<std::vector<T>, absolute> : eq_t_std_vec<T, absolute> {};

      template<class T, int n, CmpStyle cstyle>
      struct eq_t_fvec {
        typedef Dune::FieldVector<T, n> V;
        static bool eq(const V &first,
                       const V &second,
                       typename EpsilonType<V>::Type epsilon = DefaultEpsilon<V>::value()) {
          for(int i = 0; i < n; ++i)
            if(!eq_t<T, cstyle>::eq(first[i], second[i], epsilon))
              return false;
          return true;
        }
      };
      template< class T, int n >
      struct eq_t< Dune::FieldVector<T, n>, relativeWeak> : eq_t_fvec<T, n, relativeWeak> {};
      template< class T, int n >
      struct eq_t< Dune::FieldVector<T, n>, relativeStrong> : eq_t_fvec<T, n, relativeStrong> {};
      template< class T, int n >
      struct eq_t< Dune::FieldVector<T, n>, absolute> : eq_t_fvec<T, n, absolute> {};
    } // namespace Impl

    // operations in functional style
    template <class T, CmpStyle style>
    bool eq(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return Impl::eq_t<T, style>::eq(first, second, epsilon);
    }
    template <class T, CmpStyle style>
    bool ne(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return !eq<T, style>(first, second, epsilon);
    }
    template <class T, CmpStyle style>
    bool gt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return first > second && ne<T, style>(first, second, epsilon);
    }
    template <class T, CmpStyle style>
    bool lt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return first < second && ne<T, style>(first, second, epsilon);
    }
    template <class T, CmpStyle style>
    bool ge(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return first > second || eq<T, style>(first, second, epsilon);
    }
    template <class T, CmpStyle style>
    bool le(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon)
    {
      return first < second || eq<T, style>(first, second, epsilon);
    }

    // default template arguments
    template <class T>
    bool eq(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return eq<T, defaultCmpStyle>(first, second, epsilon);
    }
    template <class T>
    bool ne(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return ne<T, defaultCmpStyle>(first, second, epsilon);
    }
    template <class T>
    bool gt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return gt<T, defaultCmpStyle>(first, second, epsilon);
    }
    template <class T>
    bool lt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return lt<T, defaultCmpStyle>(first, second, epsilon);
    }
    template <class T>
    bool ge(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return ge<T, defaultCmpStyle>(first, second, epsilon);
    }
    template <class T>
    bool le(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return le<T, defaultCmpStyle>(first, second, epsilon);
    }

    // rounding operations
    namespace Impl {
      template<class I, class T, CmpStyle cstyle = defaultCmpStyle, RoundingStyle rstyle = defaultRoundingStyle>
      struct round_t;
      template<class I, class T, CmpStyle cstyle>
      struct round_t<I, T, cstyle, downward> {
        static I
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          // first get an approximation
          I lower = I(val);
          I upper;
          if(eq<T, cstyle>(T(lower), val, epsilon)) return lower;
          if(T(lower) > val) { upper = lower; lower--; }
          else upper = lower+1;
          if(le<T, cstyle>(val - T(lower), T(upper) - val, epsilon))
            return lower;
          else return upper;
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct round_t<I, T, cstyle, upward> {
        static I
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          // first get an approximation
          I lower = I(val);
          I upper;
          if(eq<T, cstyle>(T(lower), val, epsilon)) return lower;
          if(T(lower) > val) { upper = lower; lower--; }
          else upper = lower+1;
          if(lt<T, cstyle>(val - T(lower), T(upper) - val, epsilon))
            return lower;
          else return upper;
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct round_t<I, T, cstyle, towardZero> {
        static I
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          if(val > T(0))
            return round_t<I, T, cstyle, downward>::round(val, epsilon);
          else return round_t<I, T, cstyle, upward>::round(val, epsilon);
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct round_t<I, T, cstyle, towardInf> {
        static I
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          if(val > T(0))
            return round_t<I, T, cstyle, upward>::round(val, epsilon);
          else return round_t<I, T, cstyle, downward>::round(val, epsilon);
        }
      };
      template<class I, class T, CmpStyle cstyle, RoundingStyle rstyle>
      struct round_t<std::vector<I>, std::vector<T>, cstyle, rstyle> {
        static std::vector<I>
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          unsigned int size = val.size();
          std::vector<I> res(size);
          for(unsigned int i = 0; i < size; ++i)
            res[i] = round_t<I, T, cstyle, rstyle>::round(val[i], epsilon);
          return res;
        }
      };
      template<class I, class T, int n, CmpStyle cstyle, RoundingStyle rstyle>
      struct round_t<Dune::FieldVector<I, n>, Dune::FieldVector<T, n>, cstyle, rstyle> {
        static Dune::FieldVector<I, n>
        round(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          Dune::FieldVector<I, n> res;
          for(int i = 0; i < n; ++i)
            res[i] = round_t<I, T, cstyle, rstyle>::round(val[i], epsilon);
          return res;
        }
      };
    } // end namespace Impl
    template<class I, class T, CmpStyle cstyle, RoundingStyle rstyle>
    I round(const T &val, typename EpsilonType<T>::Type epsilon /*= DefaultEpsilon<T, cstyle>::value()*/)
    {
      return Impl::round_t<I, T, cstyle, rstyle>::round(val, epsilon);
    }
    template<class I, class T, CmpStyle cstyle>
    I round(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, cstyle>::value())
    {
      return round<I, T, cstyle, defaultRoundingStyle>(val, epsilon);
    }
    template<class I, class T, RoundingStyle rstyle>
    I round(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return round<I, T, defaultCmpStyle, rstyle>(val, epsilon);
    }
    template<class I, class T>
    I round(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return round<I, T, defaultCmpStyle>(val, epsilon);
    }

    // truncation
    namespace Impl {
      template<class I, class T, CmpStyle cstyle = defaultCmpStyle, RoundingStyle rstyle = defaultRoundingStyle>
      struct trunc_t;
      template<class I, class T, CmpStyle cstyle>
      struct trunc_t<I, T, cstyle, downward> {
        static I
        trunc(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          // this should be optimized away unless needed
          if(!std::numeric_limits<I>::is_signed)
            // make sure this works for all useful cases even if I is an unsigned type
            if(eq<T, cstyle>(val, T(0), epsilon)) return I(0);
          // first get an approximation
          I lower = I(val); // now |val-lower| < 1
          // make sure we're really lower in case the cast truncated to an unexpected direction
          if(T(lower) > val) lower--; // now val-lower < 1
          // check whether lower + 1 is approximately val
          if(eq<T, cstyle>(T(lower+1), val, epsilon))
            return lower+1;
          else return lower;
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct trunc_t<I, T, cstyle, upward> {
        static I
        trunc(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          I upper = trunc_t<I, T, cstyle, downward>::trunc(val, epsilon);
          if(ne<T, cstyle>(T(upper), val, epsilon)) ++upper;
          return upper;
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct trunc_t<I, T, cstyle, towardZero> {
        static I
        trunc(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          if(val > T(0)) return trunc_t<I, T, cstyle, downward>::trunc(val, epsilon);
          else return trunc_t<I, T, cstyle, upward>::trunc(val, epsilon);
        }
      };
      template<class I, class T, CmpStyle cstyle>
      struct trunc_t<I, T, cstyle, towardInf> {
        static I
        trunc(const T &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          if(val > T(0)) return trunc_t<I, T, cstyle, upward>::trunc(val, epsilon);
          else return trunc_t<I, T, cstyle, downward>::trunc(val, epsilon);
        }
      };
      template<class I, class T, CmpStyle cstyle, RoundingStyle rstyle>
      struct trunc_t<std::vector<I>, std::vector<T>, cstyle, rstyle> {
        static std::vector<I>
        trunc(const std::vector<T> &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          unsigned int size = val.size();
          std::vector<I> res(size);
          for(unsigned int i = 0; i < size; ++i)
            res[i] = trunc_t<I, T, cstyle, rstyle>::trunc(val[i], epsilon);
          return res;
        }
      };
      template<class I, class T, int n, CmpStyle cstyle, RoundingStyle rstyle>
      struct trunc_t<Dune::FieldVector<I, n>, Dune::FieldVector<T, n>, cstyle, rstyle> {
        static Dune::FieldVector<I, n>
        trunc(const Dune::FieldVector<T, n> &val,
              typename EpsilonType<T>::Type epsilon = (DefaultEpsilon<T, cstyle>::value())) {
          Dune::FieldVector<I, n> res;
          for(int i = 0; i < n; ++i)
            res[i] = trunc_t<I, T, cstyle, rstyle>::trunc(val[i], epsilon);
          return res;
        }
      };
    } // namespace Impl
    template<class I, class T, CmpStyle cstyle, RoundingStyle rstyle>
    I trunc(const T &val, typename EpsilonType<T>::Type epsilon /*= DefaultEpsilon<T, cstyle>::value()*/)
    {
      return Impl::trunc_t<I, T, cstyle, rstyle>::trunc(val, epsilon);
    }
    template<class I, class T, CmpStyle cstyle>
    I trunc(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, cstyle>::value())
    {
      return trunc<I, T, cstyle, defaultRoundingStyle>(val, epsilon);
    }
    template<class I, class T, RoundingStyle rstyle>
    I trunc(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return trunc<I, T, defaultCmpStyle, rstyle>(val, epsilon);
    }
    template<class I, class T>
    I trunc(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, defaultCmpStyle>::value())
    {
      return trunc<I, T, defaultCmpStyle>(val, epsilon);
    }
  } //namespace Dune

  // oo interface
  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  FloatCmpOps<T, cstyle_, rstyle_>::
  FloatCmpOps(EpsilonType epsilon) : epsilon_(epsilon) {}


  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  typename FloatCmpOps<T, cstyle_, rstyle_>::EpsilonType
  FloatCmpOps<T, cstyle_, rstyle_>::epsilon() const
  {
    return epsilon_;
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  void
  FloatCmpOps<T, cstyle_, rstyle_>::epsilon(EpsilonType epsilon__)
  {
    epsilon_ = epsilon__;
  }


  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  eq(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::eq<ValueType, cstyle>(first, second, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  ne(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::ne<ValueType, cstyle>(first, second, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  gt(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::gt<ValueType, cstyle>(first, second, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  lt(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::lt<ValueType, cstyle>(first, second, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  ge(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::ge<ValueType, cstyle>(first, second, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  bool FloatCmpOps<T, cstyle_, rstyle_>::
  le(const ValueType &first, const ValueType &second) const
  {
    return Dune::FloatCmp::le<ValueType, cstyle>(first, second, epsilon_);
  }


  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  template<class I>
  I FloatCmpOps<T, cstyle_, rstyle_>::
  round(const ValueType &val) const
  {
    return Dune::FloatCmp::round<I, ValueType, cstyle, rstyle_>(val, epsilon_);
  }

  template<class T, FloatCmp::CmpStyle cstyle_, FloatCmp::RoundingStyle rstyle_>
  template<class I>
  I FloatCmpOps<T, cstyle_, rstyle_>::
  trunc(const ValueType &val) const
  {
    return Dune::FloatCmp::trunc<I, ValueType, cstyle, rstyle_>(val, epsilon_);
  }

} //namespace Dune
