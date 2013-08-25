// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tbb/tbb.h>
#include <numeric>
#include <iostream>

#if __cplusplus < 201103L
// provide a fallback for compilers without lambdas
struct add
{
add(int* x)
  : _x(x)
{}

int* _x;

void operator()(int i) const
{
_x[i] = i;
}
};
#endif

int main()
{
  int x[10] = {0};
#if __cplusplus >= 201103L
  tbb::parallel_for(0,10,[&](int i){ x[i] = i; });
#else
  tbb::parallel_for(0,10,add(x));
#endif
  int result = std::accumulate(x,x+10,0);
  int expected = (9*10)/2;
  std::cout << result << " == " << expected << (result == expected ? " ok" : " ERROR") << std::endl;
  return !(result == expected);
}
