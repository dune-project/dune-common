// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if ! defined DUNE_EXPRTMPL_HH && DUNE_EXPRESSIONTEMPLATES
#define DUNE_EXPRTMPL_HH

/*
   This file is part of DUNE, a Distributed and Unified Numerics Environment
   It is distributed under the terms of the GNU Lesser General Public License version 2.1
   See COPYING at the top of the source tree for the full licence.
 */

/*! @file

   @brief This file provides expression templates for the
         «Dense Matrix and Vector Template Library» and for the
         «Iterative Solvers Template Library».
   @verbatim
   $Id$
   @endverbatim
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <complex>
#include "iteratorfacades.hh"
#include "static_assert.hh"
#include "stdstreams.hh"

struct Indent
{
  int i;
  Indent() {
    i = 0;
  }
  void operator ++ ()
  {
#ifdef DUNE_VVERBOSE
    i += 3;
#endif
  };
  void operator -- ()
  {
#ifdef DUNE_VVERBOSE
    i -= 3;
#endif
  };
};

extern Indent INDENT;

inline std::ostream & operator << (std::ostream & s, const Indent & i)
{
#ifdef DUNE_VVERBOSE
  for (int n = 0; n < i.i; n++) s << " ";
#endif
  return s;
}

namespace Dune {

  template<class V> class FlatIterator;
  template<class K, int N> class FieldVector;
  template<class K, int N, int M> class FieldMatrix;
#warning this header should not know about BCRSMatrix and BlockVector
  class ISTLAllocator;
  template<class B, class A=ISTLAllocator> class BCRSMatrix;
  template<class B, class A=ISTLAllocator> class BlockVector;

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
  // FieldType specialization for FieldVector
  template <class K, int N>
  struct FieldType< FieldVector<K,N> >
  {
    typedef K type;
  };
  // BlockType specialization for FieldVector
  template <class K, int N>
  struct BlockType< FieldVector<K,N> >
  {
    typedef K type;
  };
  // FieldType specialization for const T
  template<class T>
  struct FieldType<const T>
  {
    typedef const typename FieldType<T>::type type;
  };
  // BlockType specialization for const T
  template<class T>
  struct BlockType<const T>
  {
    typedef const typename BlockType<T>::type type;
  };

  namespace ExprTmpl {

    /* Important Classes */
    template <class V> class ConstRef;
    template <class Ex> class Expression;
    template <class A, class B, template<class> class Op> class ExBinOp;
    template <class I> class Vector;
    template <class I> class Matrix;

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

    template <class K>
    struct isEndOfExpressionRecusion< FieldVector<K,1> >
    {
      enum { value=true };
    };

    template <class K>
    struct isEndOfExpressionRecusion< std::complex<K> >
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
      size_t N() const { return ex.N(); }
      double one_norm() const { return eval_one_norm(*this); }
      double one_norm_real() const { return eval_one_norm_real(*this); }
      double two_norm() const { return sqrt(eval_two_norm2(*this)); }
      double two_norm2() const { return eval_two_norm2(*this); }
      double infinity_norm() const { return eval_infinity_norm(*this); }
      double infinity_norm_real() const { return eval_infinity_norm_real(*this); }
    private:
      Ex ex;
    };

    /**
       Vector Base Class for Expression Templates
     */
    template <class I>
    class Vector {
    public:
      explicit Vector() {}
      typedef typename BlockType<I>::type block_type;
      typedef typename FieldType<I>::type field_type;
      //! dimension of the vector space
      size_t N() const {
        return asImp().N();
      }
      double one_norm() const { return eval_one_norm(*this); }
      double one_norm_real() const { return eval_one_norm_real(*this); }
      double two_norm() const { return sqrt(eval_two_norm2(*this)); }
      double two_norm2() const { return eval_two_norm2(*this); }
      double infinity_norm() const { return eval_infinity_norm(*this); }
      double infinity_norm_real() const { return eval_infinity_norm_real(*this); }
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
#ifdef DUNE_VVERBOSE
        Dune::dvverb << INDENT << "Assign Vector from Expression\n";
#endif
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
#ifdef DUNE_VVERBOSE
        Dune::dvverb << INDENT << "Assign Vector from Vector\n";
#endif
        ++INDENT;
        for (int i=0; i<N(); ++i) { asImp()[i] = v[i]; }
        --INDENT;
        return asImp();
      }
      /*
         I& assignFrom(const Vector<block_type> & x) {
         #ifdef DUNE_VVERBOSE
         Dune::dvverb << INDENT << "Assign Vector block_type\n";
         #endif
         ++INDENT;
         for (int i=0; i < asImp().N(); i++) asImp()[i] = x;
         --INDENT;
         return asImp();
         }
       */
      I& assignFrom(field_type x) {
#ifdef DUNE_VVERBOSE
        Dune::dvverb << INDENT << "Assign Vector from field_type\n";
#endif
        ++INDENT;
        for (size_t i=0; i<N(); ++i) { asImp()[i] = x; }
        --INDENT;
        return asImp();
      }
      template <class E> Vector<I>& operator+=(const Expression<E>& x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] += x[i];
        return asImp();
      }
      template <class V> Vector<I>& operator+=(const Vector<V>& x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] += x[i];
        return asImp();
      }
      template <class E> Vector<I>& operator-=(const Expression<E>& x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] -= x[i];
        return asImp();
      }
      template <class V> Vector<I>& operator-=(const Vector<V>& x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] -= x[i];
        return asImp();
      }
      Vector<I>& operator+=(field_type x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] += x;
        return asImp();
      }
      Vector<I>& operator-=(field_type x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] -= x;
        return asImp();
      }
      Vector<I>& operator*=(field_type x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] *= x;
        return asImp();
      }
      Vector<I>& operator/=(field_type x) {
        for (size_t i=0; i < asImp().N(); i++) asImp()[i] /= x;
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
#ifdef DUNE_VVERBOSE
        Dune::dvverb << INDENT << "ConstRef->dereference " << v[i] << std::endl;
#endif
        return BlockExprImp(v[i]);
      }
      size_t N() const { return v.N(); };
      double one_norm() const { return eval_one_norm(*this); }
      double one_norm_real() const { return eval_one_norm_real(*this); }
      double two_norm() const { return sqrt(eval_two_norm2(*this)); }
      double two_norm2() const { return eval_two_norm2(*this); }
      double infinity_norm() const { return eval_infinity_norm(*this); }
      double infinity_norm_real() const { return eval_infinity_norm_real(*this); }
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

  /**
     Type Traits for row_type of Matrix
   */
  template<class M>
  struct RowType
  {
    typedef typename M::row_type type;
  };
  template<class K, int N, int M>
  struct RowType< FieldMatrix<K,N,M> >
  {
    typedef FieldVector<K,M> type;
  };
  template <class I>
  struct RowType< ExprTmpl::Matrix<I> >
  {
    typedef typename RowType<I>::type type;
  };
  // RowType specialization for const T
  template<class T>
  struct RowType<const T>
  {
    typedef const typename RowType<T>::type type;
  };

  // Matrix-Vector Multiplication
  namespace ExprTmpl {

    /**
       Matrix Base Class for Expression Templates
     */
    template <class I>
    class Matrix {
    public:
      explicit Matrix() {}
      typedef typename RowType<I>::type row_type;
      typedef typename FieldType<I>::type field_type;
      //! dimension of the vector space
      size_t N() const {
        return asImp().N();
      }
      int M() const {
        return asImp().M();
      }
      row_type & operator[] (int i) {
        return asImp()[i];
      }
      const row_type & operator[] (int i) const {
        return asImp()[i];
      }
    private:
      I & asImp() { return static_cast<I&>(*this); }
      const I & asImp() const { return static_cast<const I&>(*this); }
    };

    // Trait Structs to extract infos needed for Matrix-Vector Multiplication
    template<class M>
    struct NestedDepth
    {
      enum { value = NestedDepth<typename BlockType<M>::type>::value + 1 };
    };

    template<class K, int N, int M>
    struct NestedDepth< FieldMatrix<K,N,M> >
    {
      enum { value = 1 };
    };

    template<class Me, class M>
    struct MyDepth
    {
      enum { value = MyDepth<Me,typename BlockType<M>::type>::value+1 };
    };

    template<class Me>
    struct MyDepth<Me,Me>
    {
      enum { value = 0 };
    };

    template<class B, int i>
    struct BlockTypeN
    {
      typedef typename BlockTypeN<typename BlockType<B>::type, i-1>::type type;
    };

    template<class B>
    struct BlockTypeN<B,0>
    {
      typedef B type;
    };

    template<class B>
    struct BlockTypeN<B,-1>
    {
      typedef B type;
    };

    //! Type Traits for Vector::Iterator vs (const Vector)::ConstIterator
    template<class T>
    struct ColIteratorType
    {
      typedef typename T::ColIterator type;
    };

    template<class T>
    struct ColIteratorType<const T>
    {
      typedef typename T::ConstColIterator type;
    };

    //! Iterator class for flat sequential access to a nested Matrix Row
    template<class A>
    class FlatColIterator :
      public ForwardIteratorFacade<FlatColIterator<A>,
          typename FieldType<A>::type,
          typename FieldType<A>::type&,
          int>
    {
    public:
      typedef typename ColIteratorType<A>::type ColBlockIterator;
      typedef std::ptrdiff_t DifferenceType;
      //    typedef typename BlockIterator::DifferenceType DifferenceType;
      typedef typename BlockType<A>::type block_type;
      typedef typename FieldType<A>::type field_type;
      typedef FlatColIterator<block_type> SubBlockIterator;
      FlatColIterator(const ColBlockIterator & i, const int* _M) :
        M(_M), it(i),
        bit((*i)[(*M)].begin(), M+1),
        bend((*i)[(*M)].end(), M+1) {};
      void increment ()
      {
        ++bit;
        if (bit == bend)
        {
          ++it;
          bit = (*it)[(*M)].begin();
          bend = (*it)[(*M)].end();
        }
      }
      bool equals (const FlatColIterator & fit) const
      {
        return fit.it == it && fit.bit == bit;
      }
      const field_type& dereference() const
      {
        return *bit;
      }
      template<class V>
      const field_type& vectorentry(const Dune::ExprTmpl::Vector<V> & v) const
      {
        return bit.vectorentry(v[it.index()]);
      }
      //! return index
      DifferenceType index () const
      {
        return bit.index();
      }
      FlatColIterator operator = (const ColBlockIterator & _i)
      {
        it = _i;
        bit = (*it)[(*M)].begin();
        bend = (*it)[(*M)].end();
        return *this;
      }
    private:
      const int* M;
      ColBlockIterator it;
      SubBlockIterator bit;
      SubBlockIterator bend;
    };

    template<class K, int N, int M>
    class FlatColIterator<FieldMatrix<K,N,M> > :
      public ForwardIteratorFacade<
          FlatColIterator< FieldMatrix<K,N,M> >, K, K&, int>
    {
    public:
      typedef
      typename ColIteratorType< FieldMatrix<K,N,M> >::type ColBlockIterator;
      typedef std::ptrdiff_t DifferenceType;
      typedef K field_type;
      FlatColIterator(const ColBlockIterator & i, const int*) :
        it(i) {};
      void increment ()
      {
        ++it;
      }
      bool equals (const FlatColIterator & fit) const
      {
        return fit.it == it;
      }
      field_type& dereference() const
      {
        return *it;
      }
      const field_type& vectorentry(const FieldVector<K,M> & v) const
      {
        return v[it.index()];
      }
      //! return index
      DifferenceType index () const
      {
        return it.index();
      }
      FlatColIterator operator = (const ColBlockIterator & _i)
      {
        it = _i;
        return *this;
      }
    private:
      ColBlockIterator it;
    };

    template<class K, int N, int M>
    class FlatColIterator<const FieldMatrix<K,N,M> > :
      public ForwardIteratorFacade<
          FlatColIterator< const FieldMatrix<K,N,M> >, const K, const K&, int>
    {
    public:
      typedef
      typename ColIteratorType< const FieldMatrix<K,N,M> >::type ColBlockIterator;
      typedef std::ptrdiff_t DifferenceType;
      typedef const K field_type;
      FlatColIterator(const ColBlockIterator & i, const int*) :
        it(i) {};
      void increment ()
      {
        ++it;
      }
      bool equals (const FlatColIterator & fit) const
      {
        return fit.it == it;
      }
      field_type& dereference() const
      {
        return *it;
      }
      const field_type& vectorentry(const FieldVector<K,M> & v) const
      {
        return v[it.index()];
      }
      //! return index
      DifferenceType index () const
      {
        return it.index();
      }
      FlatColIterator operator = (const ColBlockIterator & _i)
      {
        it = _i;
        return *this;
      }
    private:
      ColBlockIterator it;
    };

    /**
       B: BlockMatrix type -> indicated the current level.
       M: ,,global'' Matrix type
       V: ,,global'' Vector type
     */
    template <class B, class Mat, class Vec>
    class MatrixMulVector
    {
    public:
      typedef typename
      BlockTypeN<MatrixMulVector<Mat,Mat,Vec>, MyDepth<B,Mat>::value-1>::type
      ParentBlockType;
      typedef
      MatrixMulVector<typename BlockType<B>::type,Mat,Vec> SubMatrixMulVector;
      typedef typename
      Dune::ExprTmpl::BlockExpression< MatrixMulVector<B,Mat,Vec> >::type
      BlockExpr;
      typedef
      typename BlockType<Mat>::type::ColIterator SubColIterator;
      typedef typename Dune::FieldType<Vec>::type field_type;
      /* constructor */
      MatrixMulVector(const Mat & _A, const Vec & _v, int* _M,
                      const ParentBlockType & _parent) :
        parent(_parent), M(_M), A(_A), v(_v) {};
      BlockExpr operator[] (int i) const {
        M[MyDepth<B,Mat>::value] = i;
        return SubMatrixMulVector(A,v,M,*this);
      }
      size_t N() const { return -1; }; //r.begin()->N(); }
      const ParentBlockType & parent;
    private:
      mutable int* M;
      const Mat & A;
      const Vec & v;
    };

    template <class Mat, class Vec>
    class MatrixMulVector<Mat,Mat,Vec>
    {
    public:
      typedef
      MatrixMulVector<typename BlockType<Mat>::type,Mat,Vec> SubMatrixMulVector;
      typedef
      typename BlockType<Mat>::type::ColIterator SubColIterator;
      typedef typename
      Dune::ExprTmpl::BlockExpression< MatrixMulVector<Mat,Mat,Vec> >::type
      BlockExpr;
      typedef typename Dune::FieldType<Vec>::type field_type;
      /* constructor */
      MatrixMulVector(const Mat & _A, const Vec & _v, int* _M) :
        M(_M), A(_A), v(_v) {};
      BlockExpr operator[] (int i) const {
        M[0] = i;
        return SubMatrixMulVector(A,v,M,*this);
      }
      size_t N() const { return -1; }; // { parent.begin().N(); }
    private:
      mutable int* M;
      const Mat & A;
      const Vec & v;
    };

    template <class K, int iN, int iM, class Mat, class Vec>
    class MatrixMulVector< FieldMatrix<K,iN,iM>, Mat, Vec >
    {
    public:
      typedef typename
      BlockTypeN<MatrixMulVector<Mat,Mat,Vec>,
          MyDepth<FieldMatrix<K,iN,iM>,Mat>::value-1>::type
      ParentBlockType;
      /* constructor */
      MatrixMulVector(const Mat & _A, const Vec & _v, int* _M,
                      const ParentBlockType & _parent) :
        parent(_parent), M(_M), A(_A), v(_v ) {};
      K operator[] (int i) const {
        K x=0;
        M[MyDepth<FieldMatrix<K,iN,iM>,Mat>::value] = i;

        FlatColIterator<const Mat> j(A[*M].begin(),M+1);
        FlatColIterator<const Mat> endj(A[*M].end(),M+1);
        for (; j!=endj; ++j)
        {
          x   += (*j) * j.vectorentry(v);
        }
        return x;
      }
      size_t N() const { return iN; };
      const ParentBlockType & parent;
    private:
      mutable int* M;
      const Mat & A;
      const Vec & v;
    };

    template <class K, int iN, int iM>
    class MatrixMulVector< FieldMatrix<K,iN,iM>, FieldMatrix<K,iN,iM>,
        FieldVector<K,iM> >
    {
    public:
      typedef FieldMatrix<K,iN,iM> Mat;
      typedef FieldVector<K,iM> Vec;
      MatrixMulVector(const Mat & _A, const Vec & _v) :
        A(_A), v(_v ){};
      K operator[] (int i) const {
        K x=0;
        typename Mat::ColIterator j = A[i].begin();
        typename Mat::ColIterator endj = A[i].end();
        for (; j!=endj; ++j)
        {
          x   += (*j) * j.vectorentry(v);
        }
        return x;
      }
      size_t N() const { return iN; };
    private:
      const Mat & A;
      const Vec & v;
    };

    template <class M, class A, class B>
    struct BlockExpression< MatrixMulVector< M, A, B > >
    {
      typedef Expression< MatrixMulVector<typename BlockType<M>::type,A,B> > type;
    };

    template <class K, int N, int M, class A, class B>
    struct BlockExpression< MatrixMulVector< FieldMatrix<K,N,M>, A, B > >
    {
      typedef K type;
    };

    template<class K, int N, int M>
    ExprTmpl::Expression<
        MatrixMulVector<FieldMatrix<K,N,M>, FieldMatrix<K,N,M>, FieldVector<K,M> > >
    operator * ( const FieldMatrix<K,N,M> & A, const FieldVector<K,M> & v )
    {
      return
        ExprTmpl::Expression<
          MatrixMulVector<FieldMatrix<K,N,M>, FieldMatrix<K,N,M>, FieldVector<K,M> > >
        (
          MatrixMulVector<FieldMatrix<K,N,M>, FieldMatrix<K,N,M>, FieldVector<K,M> >
            (A, v)
        );
    }

    // template<class BM, class BV>
    // ExprTmpl::Expression<
    //   MatrixMulVector<BCRSMatrix<BM>, BCRSMatrix<BM>, BlockVector<BV> > >
    // operator * ( const BCRSMatrix<BM> & A, const BlockVector<BV> & v )
    // {
    //   static int indizes[20];
    //   return
    //     Expression<
    //       MatrixMulVector<BCRSMatrix<BM>, BCRSMatrix<BM>, BlockVector<BV> > >
    //     (
    //       MatrixMulVector<BCRSMatrix<BM>, BCRSMatrix<BM>, BlockVector<BV> >(A, v, indizes)
    //       );
    // }

    template<class M, class V>
    ExprTmpl::Expression<
        MatrixMulVector<Matrix<M>, Matrix<M>, Vector<V> > >
    operator * ( const Matrix<M> & A, const Vector<V> & v )
    {
      static int indizes[20];
      return
        Expression<
          MatrixMulVector<Matrix<M>, Matrix<M>, Vector<V> > >
        (
          MatrixMulVector<Matrix<M>, Matrix<M>, Vector<V> >(A, v, indizes)
        );
    }

    template<class I>
    struct ColIteratorType< Matrix<I> >
    {
      typedef typename I::ColIterator type;
    };
    template<class I>
    struct ColIteratorType< const Matrix<I> >
    {
      typedef typename I::ConstColIterator type;
    };

  } // namespace ExprTmpl

  template <class B, class A, class V>
  struct FieldType< ExprTmpl::MatrixMulVector<B,A,V> >
  {
    typedef typename FieldType<V>::type type;
  };
  template <class I>
  struct BlockType< ExprTmpl::Matrix<I> >
  {
    typedef typename BlockType<I>::type type;
  };
  template <class I>
  struct FieldType< ExprTmpl::Matrix<I> >
  {
    typedef typename FieldType<I>::type type;
  };

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

  /* one norm (sum over absolute values of entries) */

#define NORM eval_one_norm
#define NORM_CODE \
  { \
    typename FieldType<A>::type val=0; \
    Dune::dvverb << INDENT << "Infinity Norm of Expression\n"; \
    ++INDENT; \
    for (size_t i=0; i<a.N(); ++i) { val += eval_one_norm(a[i]); } \
    --INDENT; \
    return val; \
  }
#define VAL_CODE { return std::abs(a); }
#include "exprtmpl/norm.inc"

  template<class K>
  inline K eval_one_norm (const std::complex<K>& c)
  {
    sqrt(c.real()*c.real() + c.imag()*c.imag());
  }

  template <class A>
  typename FieldType<A>::type
  one_norm (const A & a)
  {
    return eval_one_norm(a);
  }

  /* simplified one norm (uses Manhattan norm for complex values) */

#define NORM eval_one_norm_real
#define NORM_CODE \
  { \
    typename FieldType<A>::type val=0; \
    Dune::dvverb << INDENT << "Infinity Norm of Expression\n"; \
    ++INDENT; \
    for (size_t i=0; i<a.N(); ++i) { val += eval_one_norm_real(a[i]); } \
    --INDENT; \
    return val; \
  }
#define VAL_CODE { return std::abs(a); }
#include "exprtmpl/norm.inc"

  template<class K>
  inline K eval_one_norm_real (const std::complex<K>& c)
  {
    return eval_one_norm_real(c.real()) + eval_one_norm_real(c.imag());
  }

  template <class A>
  typename FieldType<A>::type
  one_norm_real (const A & a)
  {
    return eval_one_norm_real(a);
  }

  /* two norm sqrt(sum over squared values of entries) */

  template <class A>
  typename FieldType<A>::type
  two_norm (const A & a)
  {
    return sqrt(eval_two_norm2(a));
  }

  /* sqare of two norm (sum over squared values of entries), need for block recursion */

#define NORM eval_two_norm2
#define NORM_CODE \
  { \
    typename FieldType<A>::type val=0; \
    Dune::dvverb << INDENT << "Infinity Norm of Expression\n"; \
    ++INDENT; \
    for (size_t i=0; i<a.N(); ++i) { val += eval_two_norm2(a[i]); } \
    --INDENT; \
    return val; \
  }
#define VAL_CODE { return a*a; }
#include "exprtmpl/norm.inc"

  template<class K>
  inline K eval_two_norm2 (const std::complex<K>& c)
  {
    return c.real()*c.real() + c.imag()*c.imag();
  }

  template <class A>
  typename FieldType<A>::type
  two_norm2 (const A & a)
  {
    return eval_two_norm2(a);
  }

  /* infinity norm (maximum of absolute values of entries) */

#define NORM eval_infinity_norm
#define NORM_CODE { \
    typename FieldType<A>::type val=0; \
    Dune::dvverb << INDENT << "Infinity Norm of Expression\n"; \
    ++INDENT; \
    for (size_t i=0; i<a.N(); ++i) { val = std::max(val,eval_infinity_norm(a[i])); } \
    --INDENT; \
    return val; \
}
#define VAL_CODE { return a; }
#include "exprtmpl/norm.inc"

  template <class A>
  typename FieldType<A>::type
  infinity_norm (const A & a)
  {
    return eval_infinity_norm(a);
  }

  /* simplified infinity norm (uses Manhattan norm for complex values) */

#define NORM eval_infinity_norm_real
#define NORM_CODE { \
    typename FieldType<A>::type val=0; \
    Dune::dvverb << INDENT << "Infinity Norm of Expression\n"; \
    ++INDENT; \
    for (size_t i=0; i<a.N(); ++i) { val = std::max(val,eval_infinity_norm(a[i])); } \
    --INDENT; \
    return val; \
}
#define VAL_CODE { return std::abs(a); }
#include "exprtmpl/norm.inc"

  template<class K>
  inline K eval_infinity_norm_real (const std::complex<K>& c)
  {
    return eval_one_norm_real(c.real()) + eval_one_norm_real(c.imag());
  }

  template <class A>
  typename FieldType<A>::type
  infinity_norm_real (const A & a)
  {
    return eval_infinity_norm(a);
  }

  /* vector * vector */

  namespace ExprTmpl {

    // Vector * Vector
    template <class A>
    typename FieldType<A>::type
    operator * (const Vector<A> & a, const Vector<A> & b)
    {
      assert(a.N() == b.N());
      typename FieldType<A>::type x = 0;
      for (size_t i=0; i<a.N(); i++)
        x = a[i] * b[i];
      return x;
    }

    // Expression * Vector
    template <class A, class B>
    typename FieldType<A>::type
    operator * (const Vector<A> & a, const Expression<B> & b)
    {
      dune_static_assert((is_same<FieldType<A>,FieldType<B> >::value),
                         "Field types of both operands must match!");
      assert(a.N() == b.N());
      typename FieldType<A>::type x = 0;
      for (size_t i=0; i<a.N(); i++)
        x = a[i] * b[i];
      return x;
    }

    // Vector * Expression
    template <class A, class B>
    typename FieldType<A>::type
    operator * (const Expression<A> & a, const Vector<B> & b)
    {
      dune_static_assert((is_same<FieldType<A>,FieldType<B> >::value),
                         "Field types of both operands must match!");
      assert(a.N() == b.N());
      typename FieldType<A>::type x = 0;
      for (size_t i=0; i<a.N(); i++)
        x = a[i] * b[i];
      return x;
    }

  } // namespace ExprTmpl

} // namespace Dune

#endif // DUNE_EXPRTMPL_HH
