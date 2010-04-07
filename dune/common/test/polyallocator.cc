// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>

#include <dune/common/polyallocator.hh>

struct A
{
  virtual ~A () {}
  virtual void test ( int i ) = 0;
};


struct B
  : public A
{
  B( int i ) : k( i ) {}

  void test ( int i )
  {
    std::cout << "B( " << k << " ).test( " << i << " )" << std::endl;
  }

  int k;
};


struct C
  : public A
{
  void test ( int i )
  {
    std::cout << "C.test( " << i << " )" << std::endl;
  }
};


A *create ( int argc, char **argv, Dune::PolyAllocator &allocator )
{
  if( argc > 1 )
  {
    B *b = allocator.allocate< B >();
    allocator.construct( b, B( atoi( argv[ 1 ] ) ) );
    return b;
  }
  else
  {
    C *c = allocator.allocate< C >();
    allocator.construct( c, C() );
    return c;
  }
}


int main ( int argc, char **argv )
{
  Dune::PolyAllocator allocator;
  A *a = create( argc, argv, allocator );

  a->test( 2 );

  allocator.destroy( a );
  allocator.deallocate( a );

  return 0;
}
