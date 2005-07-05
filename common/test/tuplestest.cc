// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#include <dune/common/tuples.hh>
#include <string>
#include <iostream>
#include <vector>
using namespace Dune;

template<class T>
void test(T& tuple)
{
  float f;
  f = Element<0>::get(tuple);
  int i;
  i = Element<1>::get(tuple);
  double d;
  d = Element<2>::get(tuple);
  char c;
  c = Element<3>::get(tuple);
  std::string s;
  s = Element<4>::get(tuple);
}

int iteratorTupleTest()
{
  std::vector<int> v;

  v.push_back(0);
  v.push_back(1);
  v.push_back(2);

  typedef std::vector<int>::iterator iterator;
  typedef std::vector<int>::const_iterator const_iterator;
  typedef Tuple<iterator,const_iterator> Tuple;


  Tuple tuple(v.begin(), v.end());
  int ret=0;

  if(Element<0>::get(tuple)!= v.begin()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }
  assert(Element<0>::get(tuple) == v.begin());

  if(Element<1>::get(tuple)!= v.end()) {
    std::cerr<<"Iterator tuple construction failed!"<<std::endl;
    ret++;
  }

  assert(Element<1>::get(tuple) == v.end());

  return ret;
}

int copyTest()
{
  Tuple<float,int,double,char,std::string> tuple, tuple1(3.0,1,3.3,'c',std::string("hallo")), tuple2(tuple1);

  std::cout<<tuple1<<std::endl;
  std::cout<<tuple2<<std::endl;
  tuple=tuple1;
  std::cout<<tuple<<std::endl;

  if(tuple!=tuple1)
    return 1;
  if(tuple2!=tuple1)
    return 1;

  return 0;
}

int main(int argc, char** argv)
{
  Tuple<float,int,double,char,std::string> tuple;

  test(tuple);
  test(static_cast<const Tuple<float,int,double,char,std::string>&>(tuple));
  exit(copyTest()+iteratorTupleTest());

}
