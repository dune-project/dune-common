// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_POLYALLOCATOR_HH
#define DUNE_POLYALLOCATOR_HH

#include <cstdlib>

namespace Dune
{

  // PolyAllocator
  // -------------

  struct PolyAllocator
  {
    template< class T >
    T *create ( const T &value )
    {
      return new T( value );
    }

    template< class T >
    void destroy ( T *p )
    {
      delete p;
    }
  };



  // STLPolyAllocator
  // ----------------

  /** This PolyAllocator tries to use an STL allocator to create polymorphic
   *  objects (i.e., where the type on create differs from the type on destroy).
   *
   *  Note: It is totally unclear whether this works or is in any way conforming
   *        to the standard. See FS#766 for details.
   */
  template< class A = std::allocator< void > >
  struct STLPolyAllocator
  {
    explicit STLPolyAllocator ( const A &a = A() )
      : allocator_( a )
    {}

    template< class T >
    T *create ( const T &value )
    {
      typename A::template rebind< T >::other allocator( allocator_ );
      T *p = allocator.allocate( 1 );
      allocator.construct( p, value );
      return p;
    }

    template< class T >
    void destroy ( T *p )
    {
      typename A::template rebind< T >::other allocator( allocator_ );
      allocator.destroy( p );
      allocator.deallocate( p, 1 );
    }

  private:
    typename A::template rebind< void * >::other allocator_;
  };

}

#endif // #ifndef DUNE_POLYALLOCATOR_HH
