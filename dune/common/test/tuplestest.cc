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
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

#include <dune/common/classname.hh>
#include <dune/common/tuples.hh>

using namespace Dune;

template<class T>
void test(T& tuple)
{
  float f DUNE_UNUSED;
  f = std::get<0>(tuple);
  int i DUNE_UNUSED;
  i = std::get<1>(tuple);
  double d DUNE_UNUSED;
  d = std::get<2>(tuple);
  char c DUNE_UNUSED;
  c = std::get<3>(tuple);
  std::string s;
  s = std::get<4>(tuple);
  typename std::tuple_element<4,typename std::remove_const<T>::type>::type s2 = std::get<4>(tuple);
}

int iteratorTupleTest()
{
  std::vector<int> v;

  v.push_back(0);
  v.push_back(1);
  v.push_back(2);

  typedef std::vector<int>::iterator iterator;
  typedef std::vector<int>::const_iterator const_iterator;
  typedef std::tuple<iterator,const_iterator, const_iterator> Tuple;


  Tuple tuple_(v.begin(), v.begin(), v.end());
  static_assert(std::tuple_size<Tuple>::value==3, "The tuple size should be 3!");;

  int ret=0;

  if(std::get<0>(tuple_)!= v.begin()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }
  assert(std::get<1>(tuple_) == v.begin());
  assert(std::get<1>(tuple_) == std::get<0>(tuple_));
  if(std::get<2>(tuple_)!= v.end()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }

  assert(std::get<2>(tuple_) == v.end());
  assert(std::get<0>(tuple_) != v.end());
  assert(std::get<1>(tuple_) != std::get<2>(tuple_));
  return ret;
}

int lessTest()
{
  std::tuple<int,float,double> t1(1,2.0,3.0);
  std::tuple<int,int,int> t2(1,2,1);

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
  std::tuple<float,int,double,char,std::string> tuple_, tuple1(3.0,1,3.3,'c',std::string("hallo")), tuple2(tuple1);

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
    const std::tuple<int&> t(n);
    std::get<0>(t) = 777;
    assert(n == 777);
  }

  int k=5;
  int& kr(k);
  kr=20;
  int i=50;
  double d=-3.3;
  long j=-666;
  std::tuple<int,double,long> t1(100, 5.0, 10);
  std::tuple<int,int,int> t2(1,5,9);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  std::tuple<int&,double&,long&> tr(i,d,j);

  std::get<0>(tr)=3;
  assert(std::get<0>(tr)==3);

  std::cout <<"tr="<< tr<<std::endl;

  std::tuple<int> i1(5);
  std::tuple<int&> ir(i);
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
  std::tuple<int*,double*,long*> t1(&k, &d, &j);
  std::tuple<int*,double*,long*> t2(&k1,&d1,&j1);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  std::tuple<int*,double*,long*> tr(&i,&d,&j);

  *std::get<0>(tr)=3;
  assert(*std::get<0>(tr)==3);

  std::cout <<"tr="<< tr<<std::endl;

  std::tuple<int> i1(5);
  std::tuple<int*> ir(&i);

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
  std::tuple<const int*, const double*, const long*> t1(&k, &d, &j);
  std::tuple<int*, double*, long*> t2(&k1,&d1,&j1);
  std::cout << "i="<<i<<" d="<<d<<" j="<<j<<std::endl;

  std::tuple<const int*, const double*, const long*> tr(&i,&d,&j);

  std::cout << *std::get<0>(tr)<<std::endl;

  std::cout <<"tr="<< tr<<std::endl;

  std::tuple<int> i1(5);
  std::tuple<const int*> ir(&i);

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

  std::tuple<int,double> t(1,3.14);
  int sz = std::tuple_size<std::tuple<int, double, char> >::value;
  if(sz!=3) ++ret;

  // contruct a tuple

  t= std::make_tuple(5, 10.9);


  // get the second element
  std::tuple_element<1,std::tuple<int,double> >::type d DUNE_UNUSED;
  d=std::get<1>(t);

  std::get<0>(t)=16;

  std::cout<<t<<std::endl;

  return ret;
}

int inputTest()
{
  typedef std::tuple<int, int, int> Tuple;
  const std::string data = "1, 2, 3";
  const Tuple expected{1, 2, 3};

  std::istringstream in(data);
  Tuple t;
  in >> t;

  return t == expected ? 0 : 1;
}

int outputTest()
{
  typedef std::tuple<int, int, int> Tuple;
  const Tuple t{1, 2, 3};
  const std::string expected = "1, 2, 3";

  std::ostringstream out;
  out << t;

  return out.str() == expected ? 0 : 1;
}

int main(int, char**)
{
  std::tuple<float,int,double,char,std::string> tuple_;
  std::cout << "=== testing tuple: " << className(tuple_) << std::endl;

  test(tuple_);
  test(static_cast<std::tuple<float,int,double,char,std::string>& >(tuple_));
  test(static_cast<const std::tuple<float,int,double,char,std::string>&>(tuple_));
  return (copyTest()+iteratorTupleTest()+referenceTest()+lessTest()
          +pointerTest()+constPointerTest()+tuple_tr1_test()+inputTest()+outputTest());

}
