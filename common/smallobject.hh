// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SMALLOBJECT_HH
#define DUNE_SMALLOBJECT_HH

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
        delete list_[ i ];
    }

    static SmallObjectPool &instance ()
    {
      static SmallObjectPool inst;
      return inst;
    }

    static Block *&list ( unsigned int blocks )
    {
      return instance().list_[ blocks ];
    }

  public:
    static void *allocate ( unsigned int size )
    {
      const unsigned int blocks = (size + (blockSize-1)) / blockSize;
      if( blocks > maxBlocks )
        return 0;
      Block *blockPtr = list( blocks );
      if( blockPtr != 0 )
        list( blocks ) = blockPtr->next;
      else
        blockPtr = new Block[ blocks+1 ];
      blockPtr->blocks = blocks;
      return blockPtr+1;
    }

    static void free ( void *ptr )
    {
      if( ptr != 0 )
      {
        Block *blockPtr = reinterpret_cast< Block * >( ptr ) - 1;
        const unsigned int blocks = blockPtr->blocks;
        blockPtr->next = list( blocks );
        list( blocks ) = blockPtr;
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
