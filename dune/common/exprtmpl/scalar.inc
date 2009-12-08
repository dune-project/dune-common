// -*- C++ -*-
#ifndef OP
#error OP undefined
#endif

#ifdef ExpressionOpScalar

namespace ExprTmpl {

template <class A>
class ExpressionOpScalar
{
public:
  typedef typename FieldType<A>::type field_type;
  typedef typename BlockType<A>::type block_type;
  typedef typename BlockExpression<ExpressionOpScalar>::type BlockExpr;
  ExpressionOpScalar (const Expression<A> & _a,
            const typename FieldType<A>::type & _lambda)
    : a(_a), lambda(_lambda) {}
  BlockExpr operator[] (int i) const {
    return a[i] OP lambda;
  }
  int N() const { return a.N(); };
private:
  Expression<A> a;
  const field_type lambda;
};

// Scalar op FieldVector<K,1>
template <class K>
K operator OP (const Expression< ConstRef< FieldVector<K,1> > > & a,
               const K & lambda)
{
  return a[0] OP lambda;
}

// Vector op Scalar
template <class A>
Expression< ExpressionOpScalar<ConstRef<A> > >
operator OP (const Vector<A> & a, const typename FieldType<A>::type & lambda)
{
  ConstRef<A> ra(a);
  ExpressionOpScalar< ConstRef<A> > ex(ra, lambda);
  return Expression< ExpressionOpScalar<ConstRef<A> > >(ex);
}

// Expression op Scalar
template <class A>
Expression< ExpressionOpScalar<A> >
operator OP (const Expression<A> & a, const typename FieldType<A>::type & lambda)
{
  ExpressionOpScalar<A> ex(a, lambda);
  return Expression< ExpressionOpScalar<A> >(ex);
}

// TypeTraits
//#warning Kann hier nicht das innere ExpressionImp weg?
template <class Ex>
struct BlockExpression< ExprTmpl::ExpressionOpScalar<Ex> >
{
  typedef
  typename SelectType<
    isEndOfExpressionRecusion< typename BlockType<Ex>::type >::value,
    typename FieldType<Ex>::type,
    ExprTmpl::Expression<
    ExprTmpl::ExpressionOpScalar<typename ExpressionImp<
      typename BlockExpression<
        typename ExpressionImp<Ex>::type>::type>::type > >
  >::Type type;
};

template <class Ex>
struct ExpressionImp< ExprTmpl::ExpressionOpScalar<Ex> >
{
  typedef ExprTmpl::ExpressionOpScalar< Ex > type;
};

} // namespace ExprTmpl

template <class A>
struct FieldType< ExprTmpl::ExpressionOpScalar<A> >
{
  typedef typename FieldType<A>::type type;
};

template <class A>
struct BlockType< ExprTmpl::ExpressionOpScalar<A> >
{
  typedef typename BlockType<A>::type type;
};
  
#undef ExpressionOpScalar

#endif

#ifdef ScalarOpExpression

namespace ExprTmpl {

template <class A>
class ScalarOpExpression
{
public:
  typedef typename FieldType<A>::type field_type;
  typedef typename BlockType<A>::type block_type;
  typedef typename BlockExpression<ScalarOpExpression>::type BlockExpr;
  ScalarOpExpression (const Expression<A> & _a,
            const typename FieldType<A>::type & _lambda)
    : a(_a), lambda(_lambda) {}
  BlockExpr operator[] (int i) const {
    return lambda OP a[i];
  }
  int N() const { return a.N(); };
private:
  Expression<A> a;
  const field_type lambda;
};

// Scalar op FieldVector<K,1>
template <class K>
K operator OP (const K & lambda,
               const Expression< ConstRef< FieldVector<K,1> > > & a)
{
  return lambda OP a[0];
}

// Scalar op Vector
template <class A>
Expression< ScalarOpExpression<ConstRef<A> > >
operator OP (const typename FieldType<A>::type & lambda, const Vector<A> & a)
{
  ConstRef<A> ra(a);
  ScalarOpExpression< ConstRef<A> > ex(ra, lambda);
  return Expression< ScalarOpExpression<ConstRef<A> > >(ex);
}

// Scalar op Expression
template <class A>
Expression< ScalarOpExpression<A> >
operator OP (const typename FieldType<A>::type & lambda, const Expression<A> & a)
{
  ScalarOpExpression<A> ex(a, lambda);
  return Expression< ScalarOpExpression<A> >(ex);
}

// TypeTraits
template <class Ex>
struct BlockExpression< ExprTmpl::ScalarOpExpression<Ex> >
{
  typedef
  typename SelectType<
    isEndOfExpressionRecusion< typename BlockType<Ex>::type >::value,
    typename FieldType<Ex>::type,
    ExprTmpl::Expression<
    ExprTmpl::ScalarOpExpression<typename ExpressionImp<
      typename BlockExpression<
        typename ExpressionImp<Ex>::type>::type>::type > >
  >::Type type;
};

template <class Ex>
struct ExpressionImp< ExprTmpl::ScalarOpExpression<Ex> >
{
  typedef ExprTmpl::ScalarOpExpression< Ex > type;
};

} // namespace ExprTmpl

template <class A>
struct FieldType< ExprTmpl::ScalarOpExpression<A> >
{
  typedef typename FieldType<A>::type type;
};

template <class A>
struct BlockType< ExprTmpl::ScalarOpExpression<A> >
{
  typedef typename BlockType<A>::type type;
};
  
#undef ScalarOpExpression

#endif

#undef OP
