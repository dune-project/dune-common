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

    static void free ( void *ptr )
    {
      if( ptr != 0 )
      {
        Block *current = reinterpret_cast< Block * >( ptr ) - 1;
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
      return SmallObjectPool :: allocate( size );
    }

    void operator delete ( void *ptr )
    {
      SmallObjectPool :: free( ptr );
    }
  };

}

#endif
