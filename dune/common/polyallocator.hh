// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_POLYALLOCATOR_HH
#define DUNE_POLYALLOCATOR_HH

namespace Dune
{

  // PolyAllocator
  // -------------

  struct PolyAllocator
  {
    template< class T >
    T *allocate ( size_t n = 1 )
    {
      return static_cast< T * >( operator new( n * sizeof( T ) ) );
    }

    template< class T > void deallocate ( T *p )
    {
      operator delete( p );
    }

    template< class T > void construct ( T *p, const T &value )
    {
      new( p ) T( value );
    }

    template< class T > void destroy ( T *p )
    {
      p->~T();
    }
  };

}

#endif // #ifndef DUNE_POLYALLOCATOR_HH
