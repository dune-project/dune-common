// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/tuples.hh>
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include <cstdlib>

using namespace Dune;

template<class T>
void test(T& tuple)
{
  float f DUNE_UNUSED;
  f = Element<0>::get(tuple);
  int i DUNE_UNUSED;
  i = Element<1>::get(tuple);
  double d DUNE_UNUSED;
  d = Element<2>::get(tuple);
  char c DUNE_UNUSED;
  c = Element<3>::get(tuple);
  std::string s;
  s = Element<4>::get(tuple);

  typedef typename tuple_element<4, typename remove_const<T>::type> :: type Type4 ;
  Type4 s2 = Element<4>::get(tuple);
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
  dune_static_assert(Size<Tuple>::value==3, "The tuple size should be 3!");;

  int ret=0;

  if(Element<0>::get(tuple_)!= v.begin()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }
  assert(Element<0>::get(tuple_) == v.begin());
  assert(Element<1>::get(tuple_) == Element<0>::get(tuple_));
  if(Element<2>::get(tuple_)!= v.end()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }

  assert(Element<2>::get(tuple_) == v.end());
  assert(Element<0>::get(tuple_) != v.end());
  assert(Element<1>::get(tuple_)!= Element<2>::get(tuple_));
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

  Element<0>::get(tr)=3;
  assert(Element<0>::get(tr)==3);

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

  *Element<0>::get(tr)=3;
  assert(*Element<0>::get(tr)==3);

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

  std::cout << *Element<0>::get(tr)<<std::endl;

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


int main(int argc, char** argv)
{
  tuple<float,int,double,char,std::string> tuple_;

  test(tuple_);
  test(static_cast<tuple<float,int,double,char,std::string>& >(tuple_));
  test(static_cast<const tuple<float,int,double,char,std::string>&>(tuple_));
  return (copyTest()+iteratorTupleTest()+referenceTest()+lessTest()
          +pointerTest()+constPointerTest()+tuple_tr1_test());

}
