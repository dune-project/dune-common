// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>

#include <dune/common/timer.hh>
#include <dune/common/smallobject.hh>
#include <dune/common/poolallocator.hh>

using namespace Dune;

class A
{
  int a_;

public:
  A( int a )
    : a_( a )
  {}
};


class B
  : public SmallObject
{
  int b_;

public:
  B( int b )
    : b_( b )
  {}
};


int main ( int argc, char **argv )
{
  Timer timer;

  const unsigned long iterations = 1 << 26;
  std :: cout << "Performing " << iterations << " iterations." << std :: endl;

  timer.reset();
  for( unsigned long i = 0; i < iterations; ++i )
  {
    A *a = new A( (int)i );
    delete a;
  }
  double timeA = timer.elapsed();
  std :: cout << "Time without SmallObject: " << timeA << std :: endl;

  timer.reset();
  for( unsigned long i = 0; i < iterations; ++i )
  {
    B *b = new B( (int)i );
    delete b;
  }
  double timeB = timer.elapsed();
  std :: cout << "Time with SmallObject: " << timeB << std :: endl;
  std :: cout << "Result: SmallObject is " << (timeA / timeB) << " times faster." << std :: endl;

  timer.reset();
  PoolAllocator<B,100> pool;
  for( unsigned long i = 0; i < iterations; ++i )
  {
    B *b = pool.allocate(1);
    pool.construct(b, B((int)i ));
    pool.destroy(b);
    pool.deallocate(b,1);
  }
  double timeB2 = timer.elapsed();
  std :: cout << "Time with pool allocator: " << timeB2 << std :: endl;
  std :: cout << "Result: pool allocator is " << (timeA / timeB2) << " times faster." << std :: endl;
}
