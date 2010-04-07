// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>

#include <dune/common/polyallocator.hh>

// A test of the PolyAllocator as used in the GenericGeometries:
// struct A   -> HybridMapping
// struct B,C -> VirtualMapping, i.e., HybridMappingImpl
// struct G   -> BasicGeometry

struct A
{
  virtual ~A () {}
  virtual void test ( ) = 0;
};

struct B
  : public A
{
  B( int i ) : k( i ) {}

  void test ( )
  {
    std::cout << "B( " << k << " ).test( )" << std::endl;
  }

private:
  int k;
};

struct C
  : public A
{
  void test ( )
  {
    std::cout << "C.test( )" << std::endl;
  }
};

template <class Allocator>
struct G
{
  explicit G( int k, const Allocator &alloc = Allocator() ) :
    alloc_(alloc)
  {
    if( k>0 )
    {
      B *b = alloc_.template allocate< B >();
      alloc_.construct( b, B( k ) );
      a_ = b;
    }
    else
    {
      C *c = alloc_.template allocate< C >();
      alloc_.construct( c, C() );
      a_ = c;
    }
  }
  ~G()
  {
    alloc_.destroy( a_ );
    alloc_.deallocate( a_ );
  }
  void test()
  {
    a_->test();
  }
private:
  Allocator alloc_;
  A *a_;
};

int main ( int argc, char **argv )
{
  int k = 0;
  if( argc > 1 )
    k = atoi(argv[1]);

  {
    G<Dune::PolyAllocator> g(k);
    g.test( );
  }

  return 0;
}
