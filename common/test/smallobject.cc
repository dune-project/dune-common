// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>

#include <dune/common/timer.hh>
#include <dune/common/smallobject.hh>

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

  const unsigned long iterations = 1 << 30;
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
}
