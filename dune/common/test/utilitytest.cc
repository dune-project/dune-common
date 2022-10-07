// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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

  [[maybe_unused]] RefTuple1 refs2(Dune::transformTuple<Dune::AddRefTypeEvaluator>(t1));
  PointerTuple1 pointers1(Dune::transformTuple<Dune::AddPtrTypeEvaluator>(refs));
  if(&i != std::get<0>(pointers1) || &c != std::get<1>(pointers1) ||
     &l != std::get<2>(pointers1) || &c != std::get<3>(pointers1)) {
    std::cerr << "utilitytest: error: incorrect pointers in pointers1"
              << std::endl;
    ret = 1;
  }

  if(Dune::At<2>::get(pointers)!=std::get<1>(pointers)) {
    ret+=10;
    std::cerr<<"at inconsistent!"<<std::endl;
  }

  PointerTuple1 p(new int(), new char(), new long(), new char());

  [[maybe_unused]] typedef Dune::ForEachType<Eval,PointerTuple1>::Type ConvertedType;
  Dune::PointerPairDeletor<PointerTuple1>::apply(p);
  if(p != PointerTuple1(nullptr,nullptr,nullptr,nullptr)){
    ret+=20;
    std::cerr<<"PointerPairDeletor not working!"<<std::endl;
  }

  return ret;
}
