// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/tuples.hh>
#include <dune/common/utility.hh>
#include <iostream>



template<class T>
struct Eval
{
  typedef void* Type;
};

struct Counter {
  Counter() : result_(0) {}

  template <class T1>
  void visit(T1 elem) { ++result_; }

  template <class T1, class T2>
  void visit(T1 elem, T2 elem1) { ++(++result_); }
  int result_;
};


int main(int argc, char** argv)
{

  typedef Dune::tuple<int*,double*,long*,char*> PointerTuple;

  Dune::tuple<int*,double*,long*,char*> pointers = Dune::NullPointerInitialiser<PointerTuple>::apply();
  int ret=0;

  if(Dune::get<0>(pointers)!=0) {
    std::cerr<<"First pointer not null"<<std::endl;
    ret=1;
  }
  if(Dune::get<1>(pointers)!=0) {
    std::cerr<<"Second pointer not null"<<std::endl;
    ret=2;
  }

  if(Dune::get<2>(pointers)!=0) {
    std::cerr<<"Third pointer not null"<<std::endl;
    ret=3;
  }

  if(Dune::get<3>(pointers)!=0) {
    std::cerr<<"Fourth pointer not null"<<std::endl;
    ret=4;
  }

  int i = 3;
  int *ii; ii = &i;
  double d; d = 2;
  long l = 4;
  char c = 's';

  typedef Dune::tuple<int*,char*,long*,char*> PointerTuple1;

  PointerTuple1 pointers1(&i,&c,&l,&c);


  if(static_cast<size_t>(Dune::Length<PointerTuple>::value) != static_cast<size_t>(Dune::tuple_size<PointerTuple>::value)) {
    std::cerr<<"Length and size do not match!"<<std::endl;
  }

  Counter count;
  Dune::ForEachValue<PointerTuple> forEach(pointers);

  forEach.apply(count);
  std::cout << "Number of elements is: " << count.result_ << std::endl;



  Dune::ForEachValuePair<PointerTuple,PointerTuple1> foreach1(pointers, pointers1);

  foreach1.apply(count);

  if(Dune::At<2>::get(pointers)!=Dune::get<1>(pointers)) {
    ret+=10;
    std::cerr<<"at inconsistent!"<<std::endl;
  }

  PointerTuple1 p(new int(), new char(), new long(), new char());

  typedef Dune::ForEachType<Eval,PointerTuple1>::Type ConvertedType;
  Dune::PointerPairDeletor<PointerTuple1>::apply(p);

  return ret;
}
