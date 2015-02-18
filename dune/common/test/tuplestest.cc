// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

// make sure assert works even when not compiling for debugging
#ifdef NDEBUG
#undef NDEBUG
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <dune/common/classname.hh>
#include <dune/common/tuples.hh>

using namespace Dune;

template<class T>
void test(T& tuple)
{
  float f DUNE_UNUSED;
  f = get<0>(tuple);
  int i DUNE_UNUSED;
  i = get<1>(tuple);
  double d DUNE_UNUSED;
  d = get<2>(tuple);
  char c DUNE_UNUSED;
  c = get<3>(tuple);
  std::string s;
  s = get<4>(tuple);
  typename tuple_element<4,typename remove_const<T>::type>::type s2 = get<4>(tuple);
}

int iteratorTupleTest()
{
  std::vector<int> v;

  v.push_back(0);
  v.push_back(1);
  v.push_back(2);

  typedef std::vector<int>::iterator iterator;
  typedef std::vector<int>::const_iterator const_iterator;
  typedef tuple<iterator,const_iterator, const_iterator> Tuple;


  Tuple tuple_(v.begin(), v.begin(), v.end());
  static_assert(tuple_size<Tuple>::value==3, "The tuple size should be 3!");;

  int ret=0;

  if(get<0>(tuple_)!= v.begin()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }
  assert(get<1>(tuple_) == v.begin());
  assert(get<1>(tuple_) == get<0>(tuple_));
  if(get<2>(tuple_)!= v.end()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }

  assert(get<2>(tuple_) == v.end());
  assert(get<0>(tuple_) != v.end());
  assert(get<1>(tuple_) != get<2>(tuple_));
  return ret;
}

int lessTest()
{
  tuple<int,float,double> t1(1,2.0,3.0);
  tuple<int,int,int> t2(1,2,1);

  int ret=0;

  if ((t1<t2) != false) ret++;
  std::cout << "[" << t1 << "] < [" << t2 << "] = " << (t1<t2) << std::endl;
  if ((t2<t1) != true) ret++;
  std::cout << "[" << t2 << "] < [" << t1 << "] = " << (t2<t1) << std::endl;

  // This would result in a compiler error
  //  Tuple<int,int> t3(1,2);
  //  std::cout << "[" << t3 << "] < [" << t1 << "] = " << (t3<t1) << std::endl;

  return ret;
}

int copyTest()
{
  tuple<float,int,double,char,std::string> tuple_, tuple1(3.0,1,3.3,'c',std::string("hallo")), tuple2(tuple1);

  std::cout<<tuple1<<std::endl;
  std::cout<<tuple2<<std::endl;
  tuple_=tuple1;
  std::cout<<tuple_<<std::endl;

  if(tuple_!=tuple1)
    return 1;
  if(tuple2!=tuple1)
    return 1;

  return 0;
}

int referenceTest()
{
  // make sure const tuples of references have assignable members, as long as
  // the member references a non-const type
  {
    int n = 0;
    const tuple<int&> t(n);
    get<0>(t) = 777;
    assert(n == 777);
  }

  int k=5;
  int& kr(k);
  kr=20;
  int i=50;
  double d=-3.3;
  long j=-666;
  tuple<int,double,long> t1(100, 5.0, 10);
  tuple<int,int,int> t2(1,5,9);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  tuple<int&,double&,long&> tr(i,d,j);

  get<0>(tr)=3;
  assert(get<0>(tr)==3);

  std::cout <<"tr="<< tr<<std::endl;

  tuple<int> i1(5);
  tuple<int&> ir(i);
  ir=i1;

  t1=t2;

  std::cout <<"tr="<< tr<<std::endl;
  std::cout <<"t1="<< t1<<std::endl;
  tr=t1;

  if(tr!=t1)
    return 1;
  else
    std::cout<<"t1="<<t1<< " tr="<<tr<<std::endl;


  return 0;
}

int pointerTest()
{
  int k=5, k1=6;
  int i=50;
  double d=-3.3, d1=7.8;
  long j=-666, j1=-300;
  tuple<int*,double*,long*> t1(&k, &d, &j);
  tuple<int*,double*,long*> t2(&k1,&d1,&j1);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  tuple<int*,double*,long*> tr(&i,&d,&j);

  *get<0>(tr)=3;
  assert(*get<0>(tr)==3);

  std::cout <<"tr="<< tr<<std::endl;

  tuple<int> i1(5);
  tuple<int*> ir(&i);

  t2=t1;

  std::cout <<"tr="<< tr<<std::endl;
  std::cout <<"t1="<< t1<<std::endl;
  tr=t1;

  if(tr!=t1)
    return 1;
  else
    std::cout<<"t1="<<t1<< " tr="<<tr<<std::endl;


  return 0;
}

int constPointerTest()
{
  int k=5, k1=88;
  int i=50;
  double d=-3.3, d1=6.8;
  long j=-666, j1=-500;
  tuple<const int*, const double*, const long*> t1(&k, &d, &j);
  tuple<int*, double*, long*> t2(&k1,&d1,&j1);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  tuple<const int*, const double*, const long*> tr(&i,&d,&j);

  std::cout << *get<0>(tr)<<std::endl;

  std::cout <<"tr="<< tr<<std::endl;

  tuple<int> i1(5);
  tuple<const int*> ir(&i);

  t1=t2;

  std::cout <<"tr="<< tr<<std::endl;
  std::cout <<"t1="<< t1<<std::endl;
  tr=t1;

  if(tr!=t1)
    return 1;
  else
    std::cout<<"t1="<<t1<< " tr="<<tr<<std::endl;


  return 0;
}

int tuple_tr1_test()
{
  int ret=0;

  tuple<int,double> t(1,3.14);
  int sz = tuple_size<tuple<int, double, char> >::value;
  if(sz!=3) ++ret;

  // contruct a tuple

  t= make_tuple(5, 10.9);


  // get the second element
  tuple_element<1,tuple<int,double> >::type d DUNE_UNUSED;
  d=get<1>(t);

  get<0>(t)=16;

  std::cout<<t<<std::endl;

  return ret;
}


int main(int, char**)
{
  tuple<float,int,double,char,std::string> tuple_;
  std::cout << "=== testing tuple: " << className(tuple_) << std::endl;

  test(tuple_);
  test(static_cast<tuple<float,int,double,char,std::string>& >(tuple_));
  test(static_cast<const tuple<float,int,double,char,std::string>&>(tuple_));
  return (copyTest()+iteratorTupleTest()+referenceTest()+lessTest()
          +pointerTest()+constPointerTest()+tuple_tr1_test());

}
