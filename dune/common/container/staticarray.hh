// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STATICARRAY_HH
#define DUNE_STATICARRAY_HH

#include <cassert>
#include <memory>

#include <dune/common/nullptr.hh>

namespace Dune
{

  template< class T, class A = std::allocator< T > >
  class StaticArray
  {
    typedef StaticArray< T, A > This;

    typedef typename A::template rebind< T >::other TAllocator;

    struct Storage
      : public TAllocator
    {
      explicit Storage ( const TAllocator &allocator )
        : TAllocator( allocator ), begin( nullptr ), end( nullptr )
      {}

      typename TAllocator::pointer begin;
      typename TAllocator::pointer end;
    };

  public:
    typedef T value_type;
    typedef A allocator_type;

    typedef typename TAllocator::const_pointer const_pointer;
    typedef typename TAllocator::pointer pointer;
    typedef typename TAllocator::const_reference const_reference;
    typedef typename TAllocator::reference reference;

    typedef typename TAllocator::difference_type difference_type;
    typedef typename TAllocator::size_type size_type;

    typedef const_pointer const_iterator;
    typedef pointer iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;
    typedef std::reverse_iterator< iterator > reverse_iterator;

    explicit StaticArray ( const allocator_type &allocator = allocator_type() )
      : storage_( allocator )
    {}

    explicit StaticArray ( size_type size, const value_type &value = value_type(),
                           const allocator_type &allocator = allocator_type() )
      : storage_( allocator )
    {
      allocate( size ); construct( value );
    }

    template< class InputIterator >
    StaticArray ( InputIterator first, InputIterator last, const allocator_type &allocator = allocator_type() )
      : storage_( allocator )
    {
      allocate( std::distance( first, last ) ); construct( first, last );
    }

    StaticArray ( const This &other )
      : storage_( other.allocator() )
    {
      allocate( other.size() ); construct( other.begin(), other.end() );
    }

    ~StaticArray () { destroy(); deallocate(); }

    const This &operator= ( const This &other )
    {
      destroy(); reallocate( other.size() ); construct( other.begin(), other.end() );
      return *this;
    }

    void assign ( size_type size, const value_type &value )
    {
      destroy(); reallocate( size ); construct( value );
    }

    template< class InputIterator >
    void assign ( InputIterator first, InputIterator last )
    {
      destroy(); reallocate( std::distance( first, last ) ); construct( first, last );
    }

    void clear ()
    {
      destroy(); deallocate();
      storage_.begin = storage_.end = nullptr;
    }

    void resize ( size_type size, const value_type &value = value_type() );

    void swap ( This &other )
    {
      std::swap( allocator(), other.allocator() );
      std::swap( storage_.begin, other.storage_.begin );
      std::swap( storage_.end, other.storage_.end );
    }

    const_reference back () const { assert( !empty ); return *(storage_.end-1); }
    reference back () { assert( !empty ); return *(storage_.end-1); }
    const_reference front () const { assert( !empty ); return *storage_.begin; }
    reference front () { assert( !empty ); return *storage_.begin; }

    const_reference operator[] ( size_type i ) const { assert( i < size() ); return *(storage_.begin+i); }
    reference operator[] ( size_type i ) { assert( i < size() ); return *(storage_.begin+i); }

    const_reference at ( size_type i ) const { rangeCheck( i ); return (*this)[ i ]; }
    reference at ( size_type i ) { rangeCheck( i ); return (*this)[ i ]; }

    const_iterator begin () const { return const_iterator( storage_.begin ); }
    iterator begin () { return iterator( storage_.begin ); }
    const_iterator end () const { return const_iterator( storage_.end ); }
    iterator end () { return iterator( storage_.end ); }

    const_reverse_iterator rbegin () const { return const_reverse_iterator( end() ); }
    reverse_iterator rbegin () { return reverse_iterator( end() ); }
    const_reverse_iterator rend () const { return const_reverse_iterator( begin() ); }
    reverse_iterator rend () { return reverse_iterator( begin() ); }

    bool empty () const { return (storage_.begin == storage_.end); }
    size_type size () const { return storage_.end - storage_.begin; }

    allocator_type get_allocator () const { return allocator(); }
    size_type max_size () const { return allocator().max_size(); }

  protected:
    const TAllocator &allocator () const { return static_cast< const TAllocator & >( storage_ ); }
    TAllocator &allocator () { return static_cast< TAllocator & >( storage_ ); }

    void allocate ( size_type size )
    {
      storage_.begin = (size != size_type( 0 ) ? allocator().allocate( size ) : nullptr);
      storage_.end = storage_.begin + size;
    }

    void reallocate ( size_type size )
    {
      if( storage_.begin + size != storage_.end )
      {
        deallocate();
        allocate( size );
      }
    }

    void deallocate ()
    {
      if( storage_.begin )
        allocator().deallocate( storage_.begin, size() );
    }

    void construct ( const value_type &value )
    {
      for( pointer it = storage_.begin; it != storage_.end; ++it )
        allocator().construct( it, value );
    }

    template< class InputIterator >
    void construct ( InputIterator first, InputIterator last )
    {
      for( pointer it = storage_.begin; it != storage_.end; ++it, ++first )
        allocator().construct( it, *first );
      assert( first == last );
    }

    void destroy ()
    {
      for( pointer it = storage_.begin; it != storage_.end; ++it )
        allocator().destroy( it );
    }

  private:
    Storage storage_;
  };



  // Comparison Operators
  // --------------------

  template< class T, class A >
  inline bool operator== ( const StaticArray< T, A > &a, const StaticArray< T, A > &b )
  {
    return (a.size() == b.size()) && std::equal( a.begin(), a.end(), b.begin() );
  }

  template< class T, class A >
  inline bool operator!= ( const StaticArray< T, A > &a, const StaticArray< T, A > &b )
  {
    return !(a == b);
  }


  // Implementation of StaticArray
  // -----------------------------

  template< class T, class A >
  inline void StaticArray< T, A >::resize ( size_type size, const value_type &value )
  {
    if( storage_.begin + size != storage_.end )
    {
      const pointer oldBegin = storage_.begin;
      const pointer oldEnd = storage_.end;
      const pointer copyEnd = oldBegin + std::min( size, oldEnd - oldBegin );

      storage_.begin = allocate( size );
      storage_.end = storage_.begin + size;

      pointer it = storage_.begin;
      for( pointer oldIt = oldBegin; oldIt != copyEnd; ++it, ++oldIt )
      {
        allocator().construct( it, *oldIt );
        allocator().destroy( oldIt );
      }
      for( ; it != end; ++it )
        allocator().construct( it, value );
      for( pointer oldIt = copyEnd; oldIt != oldEnd; ++oldIt )
        allocator().destroy( oldIt );
    }
  }

} // namespace Dune

#endif // #ifndef DUNE_STATICARRAY_HH
