// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SMALLOBJECT_HH
#define DUNE_SMALLOBJECT_HH

#include <cassert>
#include <new>

namespace Dune
{

  // SmallObjectPool
  // ---------------

  class SmallObjectPool
  {
    union Block
    {
      Block *next;
      unsigned int blocks;
    };

  public:
    enum { blockSize = sizeof( Block ) };
    enum { maxBlocks = (1 << 10) - 1 };
    enum { maxSize = maxBlocks * blockSize };

  private:
    Block *list_[ maxBlocks ];

    SmallObjectPool ()
    {
      for( unsigned int i = 0; i < maxBlocks; ++i )
        list_[ i ] = 0;
    }

    ~SmallObjectPool ()
    {
      for( unsigned int i = 0; i < maxBlocks; ++i )
      {
        for( Block *next = list_[ i ]; next != 0; )
        {
          Block *current = next;
          next = current->next;
          delete[] current;
        }
      }
    }

    static SmallObjectPool &instance ()
    {
      static SmallObjectPool inst;
      return inst;
    }

    static Block *&list ( unsigned int blocks )
    {
      assert( blocks < maxBlocks );
      return instance().list_[ blocks ];
    }

  public:
    static void *allocate ( unsigned int size )
    {
      const unsigned int blocks = (size + (blockSize-1)) / blockSize;
      if( blocks >= maxBlocks )
        return 0;
      Block *&next = list( blocks );
      Block *current = next;
      if( current != 0 )
        next = current->next;
      else
        current = new Block[ blocks+1 ];
      current->blocks = blocks;
      return current + 1;
    }

    static void free ( void *p )
    {
      if( p != 0 )
      {
        Block *current = reinterpret_cast< Block * >( p ) - 1;
        const unsigned int blocks = current->blocks;
        Block *&next = list( blocks );
        current->next = next;
        next = current;
      }
    }
  };



  // SmallObject
  // -----------

  struct SmallObject
  {
    void *operator new ( size_t size )
    {
      return SmallObjectPool::allocate( size );
    }

    void operator delete ( void *p )
    {
      SmallObjectPool::free( p );
    }
  };



  // SmallObjectAllocator
  // --------------------

  template< class T >
  struct SmallObjectAllocator;

  template<>
  struct SmallObjectAllocator< void >
  {
    typedef void value_type;

    typedef void *pointer;
    typedef const void *const_pointer;

    template< class U > struct rebind { typedef SmallObjectAllocator< U > other; };
  };

  template< class T >
  struct SmallObjectAllocator
  {
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T *pointer;
    typedef const T *const_pointer;

    typedef T &reference;
    typedef const T &const_reference;

    template< class U > struct rebind { typedef SmallObjectAllocator< U > other; };

    SmallObjectAllocator () throw () {}
    template< class U > SmallObjectAllocator ( const SmallObjectAllocator< U > & ) throw () {}
    ~SmallObjectAllocator () throw () {}

    pointer address ( reference r ) const { return &r; }
    const_pointer address ( const_reference r ) const { return &r; }

    pointer allocate ( size_type n, SmallObjectAllocator< void >::const_pointer hint = 0 );
    void deallocate ( pointer p, size_type n );

    void construct ( pointer p, const T &value ) { new( p ) T( value ); }
    void destroy ( pointer p ) { p->~T(); }
  };



  // Implementation of SmallObjectAllocator
  // --------------------------------------

  template< class T >
  inline typename SmallObjectAllocator< T >::pointer
  SmallObjectAllocator< T >::allocate ( size_type n, SmallObjectAllocator< void >::const_pointer hint )
  {
    return reinterpret_cast< pointer >( SmallObjectPool::allocate( n * sizeof( T ) ) );
  }


  template< class T >
  inline void
  SmallObjectAllocator< T >::deallocate ( pointer p, size_type n )
  {
    SmallObjectPool::free( p );
  }


  template< class T >
  bool operator== ( const SmallObjectAllocator< T > &, const SmallObjectAllocator< T > & ) throw()
  {
    return true;
  }


  template< class T >
  bool operator!= ( const SmallObjectAllocator< T > &, const SmallObjectAllocator< T > & ) throw()
  {
    return false;
  }

}

#endif // #ifndef DUNE_SMALLOBJECT_HH
