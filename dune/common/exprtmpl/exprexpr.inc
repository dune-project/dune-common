// -*- C++ -*-
#ifndef OP
#error OP undefined
#endif

namespace ExprTmpl {

template <class A, class B>
class ExpressionOpExpression
{
public:
//#warning compare field_type of class A and B and choose infomation richest one!
  typedef typename FieldType<A>::type field_type;
  typedef typename BlockType<A>::type block_type;
  typedef typename BlockExpression<ExpressionOpExpression>::type BlockExpr;
  ExpressionOpExpression (const Expression<A> & _a, const Expression<B> & _b)
    : a(_a), b(_b) {
#ifdef DUNE_ISTL_WITH_CHECKING
      assert(a.N() == b.N());
#endif
  }
  BlockExpr operator[] (int i) const {
    return a[i] OP b[i];
  }
  int N() const { return a.N(); };
private:
  Expression<A> a;
  Expression<B> b;
};

// Expression op Expression
template <class A, class B>
Expression< ExpressionOpExpression<A,B> >
operator OP (const Expression<A> & a, const Expression<B> & b)
{
  ExpressionOpExpression<A,B> ex(a, b);
  return Expression< ExpressionOpExpression<A,B> >(ex);
}

// Expression op Vector
template <class A, class B>
Expression< ExpressionOpExpression<A, ConstRef<B> > >
operator OP (const Expression<A> & a, const Vector<B> & b)
{
  ConstRef<B> rb(b);
  ExpressionOpExpression<A, ConstRef<B> > ex(a, rb);
  return Expression< ExpressionOpExpression<A, ConstRef<B> > >(ex);
}

// Expression op FieldVector<K,1>
template <class A, class K>
K operator OP (const Expression<A> & a,
               const Expression< ConstRef< FieldVector<K,1> > >& b)
{
  return a OP b[0];
}

// Vector op Expression
template <class A, class B>
Expression< ExpressionOpExpression< ConstRef<A>, B> >
operator OP (const Vector<A> & a, const Expression<B> & b)
{
  ConstRef<A> ra(a);
  ExpressionOpExpression< ConstRef<A>, B> ex(ra, b);
  return Expression< ExpressionOpExpression<ConstRef<A>, B> >(ex);
}

// FieldVector<K,1> op Expression
template <class K, class B>
K operator OP (const Expression< ConstRef< FieldVector<K,1> > > & a,
               const Expression<B> & b)
{
  return a[0] OP b;
}

// Vector op Vector
template <class V>
Expression< ExpressionOpExpression< ConstRef<V>, ConstRef<V> > >
operator OP (const Vector<V> & a, const Vector<V> & b)
{
  ConstRef<V> ra(a);
  ConstRef<V> rb(b);
  ExpressionOpExpression< ConstRef<V>, ConstRef<V> > ex(ra, rb);
  return Expression< ExpressionOpExpression< ConstRef<V>, ConstRef<V> > >(ex);
}

// FieldVector<K,1> op FieldVector<K,1>
template <class K>
K operator OP (const Expression< ConstRef< FieldVector<K,1> > > & a,
               const Expression< ConstRef< FieldVector<K,1> > > & b)
{
  return a[0] OP b[0];
}

// TypeTraits
template<class A, class B>
struct BlockExpression< ExpressionOpExpression<A,B> >
{
  typedef
  typename SelectType<
    // decide whether we are at the and of the recusrion
    isEndOfExpressionRecusion<typename BlockType<A>::type>::value,
    // select FieldType if where are at the end of the recursion
    typename FieldType<A>::type,
    // construct nested expression otherwise
    ExprTmpl::Expression<
    ExprTmpl::ExpressionOpExpression<
      typename ExpressionImp< typename BlockExpression<
        typename ExpressionImp<A>::type>::type>::type,
      typename ExpressionImp< typename BlockExpression<
        typename ExpressionImp<B>::type>::type>::type > >
  >::Type type;
};

template <class A, class B>
struct ExpressionImp< ExpressionOpExpression<A,B> >
{
  typedef ExpressionOpExpression<A,B> type;
};

}

template <class A, class B>
struct FieldType< ExprTmpl::ExpressionOpExpression<A,B> >
{
  typedef typename FieldType<A>::type type;
};
  
template <class A, class B>
struct BlockType< ExprTmpl::ExpressionOpExpression<A,B> >
{
  typedef typename BlockType<A>::type type;
};
  
#undef ExpressionOpExpression
#undef OP
