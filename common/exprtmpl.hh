// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_EXPRTMPL_HH
#define DUNE_EXPRTMPL_HH

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include "stdstreams.hh"

struct Indent
{
  int i;
  Indent() {
    i = 0;
  }
  void operator ++ ()
  {
    i += 3;
  };
  void operator -- ()
  {
    i -= 3;
  };
};

extern Indent INDENT;

std::ostream & operator << (std::ostream & s, const Indent & i)
{
  for (int n = 0; n < i.i; n++) s << " ";
  return s;
}

namespace Dune {

  template<class V> class FlatIterator;
  template<class K, int N> class FieldVector;

  /**
     Type Traits for field_type and block_type
   */
  template<class T>
  struct BlockType
  {};
  template<class T>
  struct FieldType
  {};
  // FieldType specializations for basic data types
  template<>
  struct FieldType<double>
  {
    typedef double type;
  };
  template<>
  struct FieldType<float>
  {
    typedef float type;
  };
  template<>
  struct FieldType<int>
  {
    typedef int type;
  };
  // FieldType specialization for FieldMatrix
  template <class K, int N>
  struct FieldType< FieldVector<K,N> >
  {
    typedef K type;
  };
  // BlockType specialization for FieldMatrix
  template <class K, int N>
  struct BlockType< FieldVector<K,N> >
  {
    typedef K type;
  };
  // FieldType specialization for const T
  template<class T>
  struct BlockType<const T>
  {
    typedef const typename BlockType<T>::type type;
  };
  // BlockType specialization for const T
  template<class T>
  struct FieldType<const T>
  {
    typedef const typename FieldType<T>::type type;
  };

  namespace ExprTmpl {

    /* Important Classes */
    template <class V> class ConstRef;
    template <class Ex> class Expression;
    template <class A, class B, template<class> class Op> class ExBinOp;
    template <class I> class Vector;

    /**
       Type Trait for nested Expression
     */
    template<class T>
    struct BlockExpression
    {};
    /**
       Type Trait for Implementation of an Expression
     */
    template<class T>
    struct ExpressionImp
    {};

    template <class V>
    struct ExpressionImp< ExprTmpl::ConstRef<V> >
    {
      typedef ExprTmpl::ConstRef<V> type;
    };

    // TypeTraits
    template <>
    struct ExpressionImp<double>
    {
      typedef double type;
    };
    template <>
    struct ExpressionImp<float>
    {
      typedef float type;
    };
    template <>
    struct ExpressionImp<int>
    {
      typedef int type;
    };
    template <class Ex>
    struct ExpressionImp< Expression<Ex> >
    {
      typedef Ex type;
    };

    /** Identify End of Expression Recursion */
    template <class V>
    struct isEndOfExpressionRecusion
    {
      enum { value=false };
    };

    template <>
    struct isEndOfExpressionRecusion<double>
    {
      enum { value=true };
    };

    template <>
    struct isEndOfExpressionRecusion<int>
    {
      enum { value=true };
    };

    template <>
    struct isEndOfExpressionRecusion<float>
    {
      enum { value=true };
    };

    /** Nested Expression in a ConstRef<V> */
    template <class V>
    struct BlockExpression< ConstRef<V> >
    {
      typedef ExprTmpl::Expression<
          ExprTmpl::ConstRef<typename BlockType<V>::type> > type;
    };

    /** No nested Expression in ConstRef<FieldVector> */
    template<class K, int N>
    struct BlockExpression< ConstRef< FieldVector<K,N> > >
    {
      typedef K type;
    };

    /**
       Wrapper for an expression template
     */
    template <class Ex>
    class Expression
    {
    public:
      Expression(const Ex & x) : ex(x) {}
      typedef typename FieldType<Ex>::type field_type;
      typedef typename BlockExpression<Ex>::type BlockExpr;
      BlockExpr operator[] ( int i ) const {
        return ex[i];
      }
      int N() const { return ex.N(); }
    private:
      Ex ex;
    };

    /**
       1 Dimensional Vector Base Class
       for Glommable Expr1 Templates
     */
    template <class I>
    class Vector {
    public:
      explicit Vector() {}
      typedef typename BlockType<I>::type block_type;
      typedef typename FieldType<I>::type field_type;
      //! dimension of the vector space
      int N() const {
        return asImp().N();
      }
      block_type & operator[] (int i) {
        return asImp()[i];
      }
      const block_type & operator[] (int i) const {
        return asImp()[i];
      }
      //! begin for FlatIterator
      FlatIterator<I> fbegin() { return FlatIterator<I>(asImp().begin()); }
      //! end for FlatIterator
      FlatIterator<I> fend() { return FlatIterator<I>(asImp().end()); }
      //! begin for ConstFlatIterator
      FlatIterator<const I> fbegin() const {
        return FlatIterator<const I>(asImp().begin());
      }
      //! end for ConstFlatIterator
      FlatIterator<const I> fend() const {
        return FlatIterator<const I>(asImp().end());
      }
      //! assign Vector from Expression
      template <class E> I& assignFrom(Expression<E>& x) {
#warning should there be a resize?
#ifdef DUNE_ISTL_WITH_CHECKING
        assert(N() == x.N());
#endif
        Dune::dvverb << INDENT << "Assign Vector from Expression\n";
        ++INDENT;
        for (int i=0; i<N(); ++i) { asImp()[i] = x[i]; }
        --INDENT;
        return asImp();
      }
      //! assign Vector from Vector
      template <class V> I& assignFrom(const Vector<V>& v) {
#ifdef DUNE_ISTL_WITH_CHECKING
        assert(N() == v.N());
#endif
        Dune::dvverb << INDENT << "Assign Vector from Vector\n";
        ++INDENT;
        for (int i=0; i<N(); ++i) { asImp()[i] = v[i]; }
        --INDENT;
        return asImp();
      }
      /*
         I& assignFrom(const Vector<block_type> & x) {
         Dune::dvverb << INDENT << "Assign Vector block_type\n";
         ++INDENT;
         for (int i=0; i < asImp().N(); i++) asImp()[i] = x;
         --INDENT;
         return asImp();
         }
       */
      I& assignFrom(field_type x) {
        Dune::dvverb << INDENT << "Assign Vector from field_type\n";
        ++INDENT;
        for (int i=0; i<N(); ++i) { asImp()[i] = x; }
        --INDENT;
        return asImp();
      }
#if 0
#warning rewrite these!
      template <class E> Vector<I>& operator+=(const Expression<E>& x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] += x(i);
        return asImp();
      }
      template <class V> Vector<I>& operator+=(const Vector<V>& x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] += x(i);
        return asImp();
      }
      template <class E> Vector<I>& operator-=(const Expression<E>& x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] -= x(i);
        return asImp();
      }
      template <class V> Vector<I>& operator-=(const Vector<V>& x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] -= x(i);
        return asImp();
      }
#endif
      Vector<I>& operator+=(field_type x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] += x;
        return asImp();
      }
      Vector<I>& operator-=(field_type x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] -= x;
        return asImp();
      }
      Vector<I>& operator*=(field_type x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] *= x;
        return asImp();
      }
      Vector<I>& operator/=(field_type x) {
        for (int i=0; i < asImp().N(); i++) asImp()[i] /= x;
        return asImp();
      }
    private:
      I & asImp() { return static_cast<I&>(*this); }
      const I & asImp() const { return static_cast<const I&>(*this); }
    };

    template <class V>
    class ConstRef
    {
    public:
      typedef typename FieldType<V>::type field_type;
      typedef typename BlockType<V>::type block_type;
      typedef typename BlockExpression< ConstRef<V> >::type BlockExpr;
      typedef typename ExpressionImp<BlockExpr>::type BlockExprImp;
      ConstRef (const Vector<V> & _v) : v(_v) {}
      BlockExpr operator[] (int i) const {
        Dune::dvverb << INDENT << "ConstRef->dereference " << v[i] << std::endl;
        return BlockExprImp(v[i]);
      }
      int N() const { return v.N(); };
    private:
      const Vector<V> & v;
    };

  } // namespace ExprTmpl

  template <class A>
  struct FieldType< ExprTmpl::Expression<A> >
  {
    typedef typename FieldType<A>::type type;
  };

  template <class V>
  struct FieldType< ExprTmpl::ConstRef<V> >
  {
    typedef typename FieldType<V>::type type;
  };

  template <class I>
  struct FieldType< ExprTmpl::Vector<I> >
  {
    typedef typename FieldType<I>::type type;
  };

  // ----
  template <class A>
  struct BlockType< ExprTmpl::Expression<A> >
  {
    typedef typename BlockType<A>::type type;
  };

  template <class V>
  struct BlockType< ExprTmpl::ConstRef<V> >
  {
    typedef typename BlockType<V>::type type;
  };

  template <class I>
  struct BlockType< ExprTmpl::Vector<I> >
  {
    typedef typename BlockType<I>::type type;
  };

  // ---
  namespace ExprTmpl {

    // ---

  }

  // OPERATORS

  /* Scalar-Expression Operator */
#define OP *
#define ExpressionOpScalar ExpressionMulScalar
#define ScalarOpExpression ScalarMulExpression
#include "exprtmpl/scalar.inc"

#define OP /
#define ExpressionOpScalar ExpressionDivScalar
#include "exprtmpl/scalar.inc"

#define OP +
#define ExpressionOpScalar ExpressionAddScalar
#define ScalarOpExpression ScalarAdExpression
#include "exprtmpl/scalar.inc"

#define OP -
#define ExpressionOpScalar ExpressionMinScalar
#define ScalarOpExpression ScalarMinExpression
#include "exprtmpl/scalar.inc"

  /* Expression-Expression Operator */
#define OP +
#define ExpressionOpExpression ExpressionAddExpression
#include "exprtmpl/exprexpr.inc"

#define OP -
#define ExpressionOpExpression ExpressionMinExpression
#include "exprtmpl/exprexpr.inc"

} // namespace Dune

#endif // DUNE_EXPRTMPL_HH
