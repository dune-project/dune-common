// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <tuple>

#include <dune/common/typetraits.hh>
#include <dune/common/tupleutility.hh>

template<class T>
struct Eval
{
  typedef void* Type;
};

struct Counter {
  Counter() : result_(0) {}

  template <class T1>
  void visit(T1) { ++result_; }

  template <class T1, class T2>
  void visit(T1, T2) { ++(++result_); }
  int result_;
};

int main(int, char**)
{
  typedef std::tuple<int*,double*,long*,char*> PointerTuple;
  PointerTuple pointers = Dune::NullPointerInitialiser<PointerTuple>::apply();

  int ret=0;

  if(std::get<0>(pointers)!=nullptr) {
    std::cerr<<"First pointer not null"<<std::endl;
    ret=1;
  }
  if(std::get<1>(pointers)!=nullptr) {
    std::cerr<<"Second pointer not null"<<std::endl;
    ret=2;
  }

  if(std::get<2>(pointers)!=nullptr) {
    std::cerr<<"Third pointer not null"<<std::endl;
    ret=3;
  }

  if(std::get<3>(pointers)!=nullptr) {
    std::cerr<<"Fourth pointer not null"<<std::endl;
    ret=4;
  }

  int i = 3;
  long l = 4;
  char c = 's';

  typedef std::tuple<int,char,long,char> Tuple1;
  typedef std::tuple<int&,char&,long&,char&> RefTuple1;
  typedef std::tuple<int*,char*,long*,char*> PointerTuple1;
  static_assert((std::is_same<PointerTuple1,
                          Dune::ForEachType<Dune::AddPtrTypeEvaluator,
                              RefTuple1>::Type>::value),
                     "RefTuple1 with added pointers should be the same as "
                     "PointerTuple1, but it isn't!");

  Tuple1 t1(i,c,l,c);
  RefTuple1 refs(i, c, l, c);

#if 0
  // Broken with Debian 8 and gcc. See issue #53 on the bug tracker.
  DUNE_UNUSED RefTuple1 refs2(Dune::transformTuple<Dune::AddRefTypeEvaluator>(t1));
  PointerTuple1 pointers1(Dune::transformTuple<Dune::AddPtrTypeEvaluator>(refs));
  if(&i != std::get<0>(pointers1) || &c != std::get<1>(pointers1) ||
     &l != std::get<2>(pointers1) || &c != std::get<3>(pointers1)) {
    std::cerr << "utilitytest: error: incorrect pointers in pointers1"
              << std::endl;
    ret = 1;
  }
#endif

  PointerTuple1 pointers1 = Dune::NullPointerInitialiser<PointerTuple1>::apply();

  if(static_cast<size_t>(std::tuple_size<PointerTuple>::value) != static_cast<size_t>(std::tuple_size<PointerTuple>::value)) {
    std::cerr<<"Length and size do not match!"<<std::endl;
  }

  Counter count;
  Dune::ForEachValue<PointerTuple> forEach(pointers);

  forEach.apply(count);
  std::cout << "Number of elements is: " << count.result_ << std::endl;

  Dune::ForEachValuePair<PointerTuple,PointerTuple1> foreach1(pointers, pointers1);

  foreach1.apply(count);

  if(Dune::At<2>::get(pointers)!=std::get<1>(pointers)) {
    ret+=10;
    std::cerr<<"at inconsistent!"<<std::endl;
  }

  PointerTuple1 p(new int(), new char(), new long(), new char());

  typedef Dune::ForEachType<Eval,PointerTuple1>::Type ConvertedType DUNE_UNUSED;
  Dune::PointerPairDeletor<PointerTuple1>::apply(p);
  if(p != PointerTuple1(nullptr,nullptr,nullptr,nullptr)){
    ret+=20;
    std::cerr<<"PointerPairDeletor not working!"<<std::endl;
  }

  return ret;
}
