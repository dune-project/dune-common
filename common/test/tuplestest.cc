// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#include <dune/common/tuples.hh>
#include <string>

using namespace Dune;

template<class T>
void test(T& tuple)
{
  float f = Element<0>::get(tuple);
  int i = Element<1>::get(tuple);
  double d = Element<2>::get(tuple);
  char c = Element<3>::get(tuple);
  std::string s = Element<4>::get(tuple);
}

int main(int argc, char** argv)
{
  Tuple<float,int,double,char,std::string> tuple;

  test(tuple);
  test(static_cast<const Tuple<float,int,double,char,std::string>&>(tuple));
}
