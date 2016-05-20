// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// #define DEBUG_ALLOCATOR_KEEP 1
#define DEBUG_NEW_DELETE 3

#include <dune/common/debugallocator.hh>

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <vector>

class A
{
public:
  A() { std::cout << "INIT A\n"; }
  int x;
  void foo() {};
};

void basic_tests ()
{
  using Dune::DebugMemory::alloc_man;

  size_t s = 256;
  double * x = alloc_man.allocate<double>(s);
  x[s-1] = 10;

  // access out of bounds
#ifdef FAILURE1
  x[s+1] = 1;
#endif

  // lost allocation, free and double-free
#ifndef FAILURE2
  alloc_man.deallocate<double>(x);
#endif
#ifdef FAILURE3
  alloc_man.deallocate<double>(x);
#endif

  // access after free
#ifdef FAILURE4
  x[s-1] = 10;
#endif
}

void allocator_tests()
{
  std::vector<double, Dune::DebugAllocator<double> > v;
  v.push_back(10);
  v.push_back(12);
  v.size();
  std::cout << v[0] << "\n";
  std::cout << v[1] << "\n";
#ifdef FAILURE5
  std::cout << v[v.capacity()] << "\n";
#endif
}

void new_delete_tests()
{
  std::cout << "alloc double[3]\n";
  double * y = new double[3];
  delete[] y;

  std::cout << "alloc A[2]\n";
  A * z = new A[2];
  z->foo();
  delete[] z;
  z = 0;

  std::cout << "alloc (buf) A[3]\n";
  char * buf = (char*)malloc(128);
  A * z2 = new (buf) A[3];
  z2->foo();
  free(buf);
  z2 = 0;

  std::cout << "alloc A[4]\n";
  A * z4 = ::new A[4];
  z4->foo();
  ::delete[] z4;
  z4 = 0;
}

int main(int, char**)
{
#if EXPECTED_SIGNAL
  std::signal(EXPECTED_SIGNAL, std::_Exit);
#endif
#if EXPECTED_ALT_SIGNAL
  std::signal(EXPECTED_ALT_SIGNAL, std::_Exit);
#endif

  basic_tests();
  allocator_tests();
  new_delete_tests();
#ifdef EXPECTED_SIGNAL
  return 1;
#else
  return 0;
#endif
}
