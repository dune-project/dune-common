// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>
#include <memory>

#include <dune/common/poolallocator.hh>
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
      a_ = alloc_.create( B( k ) );
    else
      a_ = alloc_.create( C() );
  }

  ~G()
  {
    alloc_.destroy( a_ );
  }

  void test()
  {
    a_->test();
  }

private:
  Allocator alloc_;
  A *a_;
};

template< class Allocator >
struct H
{
  explicit H ( int k, const Allocator &alloc = Allocator() )
    : alloc_( alloc )
  {
    if( k > 0 )
      create< B >( B( k ) );
    else
      create< C >( C() );
  }

  ~H()
  {
    destroy_( *this );
  }

  void test()
  {
    a_->test();
  }

private:
  template< class Impl >
  void create ( const Impl &impl )
  {
    typename Allocator::template rebind< Impl >::other alloc( alloc_ );
    Impl *p = alloc.allocate( 1 );
    alloc.construct( p, impl );
    a_ = p;
    destroy_ = destroy< Impl >;
  }

  template< class Impl >
  static void destroy ( H &h )
  {
    typename Allocator::template rebind< Impl >::other alloc( h.alloc_ );
    alloc.destroy( (Impl*)h.a_ );
    alloc.deallocate( (Impl*)h.a_, 1 );
  }

  typedef void (*Destroy)( H &h );

  Allocator alloc_;
  A *a_;
  Destroy destroy_;
};

int main ( int argc, char **argv )
{
  int k = 0;
  if( argc > 1 )
    k = atoi(argv[1]);

  {
    G<Dune::PolyAllocator> g(k);
    g.test();
  }

  {
    H< std::allocator< A > > h( k );
    h.test();
  }

  if( argc > 2 )
  {
    const int numLoops = atoi( argv[ 2 ] );
    for( int i = 0; i < numLoops; ++i )
    {
      G< Dune::STLPolyAllocator< Dune::PoolAllocator< A, 4096 > > > g( k );
      g.test();
    }
  }

  return 0;
}
