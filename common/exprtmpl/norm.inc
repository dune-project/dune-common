// -*- C++ -*-
#ifndef NORM
#error NORM undefined
#endif

#ifndef NORM_CODE
#error NORM_CODE undefined
#endif

#ifndef VAL_CODE
#error VAL_CODE undefined
#endif

template <class A>
typename FieldType<A>::type
NORM (const ExprTmpl::Expression<A> & a)
{
  NORM_CODE
}

template <class A>
typename FieldType<A>::type
NORM (const ExprTmpl::Vector<A> & a)
{
  NORM_CODE
}

inline double
NORM (const double & a)
{
  VAL_CODE
}

inline float
NORM (const float & a)
{
  VAL_CODE
}

inline int
NORM (const int & a)
{
  VAL_CODE
}

#undef NORM
#undef NORM_CODE
#undef VAL_CODE
