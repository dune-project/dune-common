// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DYNAMICARRAY_HH
#define DUNE_DYNAMICARRAY_HH

#include <cassert>
#include <cmath>
#include <memory>

#include <dune/common/exceptions.hh>
#include <dune/common/nullptr.hh>

namespace Dune
{

  // CapacityManager
  // ---------------

  template< class sz_t = std::size_t >
  class CapacityManager
  {
    typedef CapacityManager< sz_t > This;

  public:
    typedef sz_t size_type;

    template< class _sz_t >
    struct rebind { typedef CapacityManager< _sz_t > other; };

    CapacityManager () : capacity_( 0 ) {}

    template< class _sz_t >
    CapacityManager ( const CapacityManager< _sz_t > & ) : capacity_( 0 ) {}
    CapacityManager ( const This & ) : capacity_( 0 ) {}

    template< class _sz_t >
    const This &operator= ( const CapacityManager< _sz_t > & ) { return *this; }
    const This &operator= ( const This & ) { return *this; }

    size_type capacity ( size_type current_size ) const { return capacity_; }

    std::pair< size_type, size_type >
    reserve ( size_type current_size, size_type desired_capacity )
    {
      const size_type current_capacity = capacity_;
      capacity_ = std::max( desired_capacity, capacity_ );
      return std::make_pair( current_capacity, capacity_ );
    }

    std::pair< size_type, size_type >
    resize ( size_type current_size, size_type desired_size )
    {
      if( capacity_ == size_type( 0 ) )
        return std::make_pair( size_type( 0 ), capacity_ = desired_size );

      const size_type current_capacity = capacity_;
      while( desired_size > capacity_ )
        capacity_ *= size_type( 2 );
      return std::make_pair( current_capacity, capacity_ );
    }

  private:
    size_type capacity_;
  };



  // StaticCapacityManager
  // ---------------------

  template< class sz_t = std::size_t >
  class StaticCapacityManager
  {
    typedef StaticCapacityManager< sz_t > This;

  public:
    typedef sz_t size_type;

    template< class _sz_t >
    struct rebind { typedef StaticCapacityManager< _sz_t > other; };

    StaticCapacityManager () {}

    template< class _sz_t >
    StaticCapacityManager ( const StaticCapacityManager< _sz_t > & ) {}
    StaticCapacityManager ( const This & ) {}

    template< class _sz_t >
    const This &operator= ( const StaticCapacityManager< _sz_t > & ) { return *this; }
    const This &operator= ( const This & ) { return *this; }

    size_type capacity ( size_type current_size ) const { return current_size; }

    std::pair< size_type, size_type >
    reserve ( size_type current_size, size_type desired_capacity )
    {
      return std::make_pair( current_size, current_size );
    }

    std::pair< size_type, size_type >
    resize ( size_type current_size, size_type desired_size )
    {
      return std::make_pair( current_size, desired_size );
    }
  };



  // AdaptiveCapacityManager
  // -----------------------

  template< class sz_t = std::size_t >
  class AdaptiveCapacityManager
  {
    typedef AdaptiveCapacityManager< sz_t > This;

  public:
    typedef sz_t size_type;

    template< class _sz_t >
    struct rebind { typedef AdaptiveCapacityManager< _sz_t > other; };

    AdaptiveCapacityManager ( const double factor = 1.125 )
      : capacity_( 0 ), factor_( factor )
    {
      assert( factor_ >= 1.0 );
    }

    template< class _sz_t >
    AdaptiveCapacityManager ( const AdaptiveCapacityManager< _sz_t > &other )
      : capacity_( 0 ), factor_( other.factor_ )
    {}

    AdaptiveCapacityManager ( const This &other )
      : capacity_( 0 ), factor_( other.factor_ )
    {}

    template< class _sz_t >
    const This &operator= ( const AdaptiveCapacityManager< _sz_t > &other )
    {
      capacity_ = 0;
      factor_ = other.factor_;
      return *this;
    }

    const This &operator= ( const This &other )
    {
      capacity_ = 0;
      factor_ = other.factor_;
      return *this;
    }

    size_type capacity ( size_type current_size ) const { return capacity_; }

    std::pair< size_type, size_type >
    reserve ( size_type current_size, size_type desired_capacity )
    {
      const size_type current_capacity = capacity_;
      if( capacity_ < desired_capacity )
        capacity_  = (size_type) std::ceil( factor_ * desired_capacity );
      return std::make_pair( current_capacity, capacity_ );
    }

    std::pair< size_type, size_type >
    resize ( size_type current_size, size_type desired_size )
    {
      const size_type overEstimate = (size_type) std::ceil( factor_ * desired_size );
      if( (desired_size <= capacity_) && (overEstimate >= capacity_) )
        return std::make_pair( capacity_, capacity_ );

      const size_type current_capacity = capacity_;
      capacity_ = overEstimate;
      return std::make_pair( current_capacity, capacity_ );
    }

  private:
    size_type capacity_;
    double factor_;
  };



  // DynamicArray
  // ------------

  template< class T, class CM = CapacityManager<>, class A = std::allocator< T > >
  class DynamicArray
  {
    typedef DynamicArray< T, CM, A > This;

    typedef typename A::template rebind< T >::other my_allocator_type;

  public:
    typedef T value_type;
    typedef CM capacity_manager_type;
    typedef A allocator_type;

    typedef typename my_allocator_type::const_pointer const_pointer;
    typedef typename my_allocator_type::pointer pointer;
    typedef typename my_allocator_type::const_reference const_reference;
    typedef typename my_allocator_type::reference reference;

    typedef typename my_allocator_type::difference_type difference_type;
    typedef typename my_allocator_type::size_type size_type;

    typedef const_pointer const_iterator;
    typedef pointer iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;
    typedef std::reverse_iterator< iterator > reverse_iterator;

  private:
    typedef typename CM::template rebind< size_type >::other my_capacity_manager_type;

    struct Storage
      : public my_capacity_manager_type
    {
      explicit Storage ( const my_capacity_manager_type &capacity_manager )
        : my_capacity_manager_type( capacity_manager ), begin( nullptr ), end( nullptr )
      {}

      pointer begin, end;
    };

    struct Impl
      : public my_allocator_type
    {
      Impl ( const my_capacity_manager_type &capacity_manager, const my_allocator_type &allocator )
        : my_allocator_type( allocator ), storage( capacity_manager )
      {}

      Storage storage;
    };

  public:
    explicit DynamicArray ( const capacity_manager_type &capacity_manager = capacity_manager_type(),
                            const allocator_type &allocator = allocator_type() )
      : impl_( capacity_manager, allocator )
    {
      initialize( size_type( 0 ) );
    }

    explicit DynamicArray ( size_type size, const value_type &value = value_type(),
                            const capacity_manager_type &capacity_manager = capacity_manager_type(),
                            const allocator_type &allocator = allocator_type() )
      : impl_( capacity_manager, allocator )
    {
      initialize( size ); construct( value );
    }

    template< class InputIterator >
    DynamicArray ( InputIterator first, InputIterator last,
                   const capacity_manager_type &capacity_manager = capacity_manager_type(),
                   const allocator_type &allocator = allocator_type() )
      : impl_( capacity_manager, allocator )
    {
      initialize( std::distance( first, last ) ); construct( first, last );
    }

    DynamicArray ( const This &other )
      : impl_( other.capacity_manager(), other.allocator() )
    {
      initialize( other.size() ); construct( other.begin(), other.end() );
    }

    ~DynamicArray () { destroy(); deallocate( capacity_manager().capacity( size() ) ); }

    const This &operator= ( const This &other )
    {
      destroy();
      capacity_manager() = other.capacity_manager();
      initialize( other.size() );
      construct( other.begin(), other.end() );
      return *this;
    }

    void assign ( size_type size, const value_type &value )
    {
      reinitialize( size );
      construct( value );
    }

    template< class InputIterator >
    void assign ( InputIterator first, InputIterator last )
    {
      reinitialize( std::distance( first, last ) );
      construct( first, last );
    }

    void clear () { reinitialize( size_type( 0 ) ); }

    void swap ( This &other )
    {
      std::swap( storage().begin, other.storage().begin );
      std::swap( storage().end, other.storage().end );
      std::swap( capacity_manager(), other.capacity_manager() );
      std::swap( allocator(), other.allocator() );
    }

    const_reference back () const { assert( !empty ); return *(storage().end-1); }
    reference back () { assert( !empty ); return *(storage.end-1); }
    const_reference front () const { assert( !empty ); return *storage.begin; }
    reference front () { assert( !empty ); return *storage.begin; }

    const_reference operator[] ( size_type i ) const { assert( i < size() ); return *(storage().begin+i); }
    reference operator[] ( size_type i ) { assert( i < size() ); return *(storage().begin+i); }

    const_reference at ( size_type i ) const { rangeCheck( i ); return (*this)[ i ]; }
    reference at ( size_type i ) { rangeCheck( i ); return (*this)[ i ]; }

    const_iterator begin () const { return const_iterator( storage().begin ); }
    iterator begin () { return iterator( storage().begin ); }
    const_iterator end () const { return const_iterator( storage().end ); }
    iterator end () { return iterator( storage().end ); }

    const_reverse_iterator rbegin () const { return const_reverse_iterator( end() ); }
    reverse_iterator rbegin () { return reverse_iterator( end() ); }
    const_reverse_iterator rend () const { return const_reverse_iterator( begin() ); }
    reverse_iterator rend () { return reverse_iterator( begin() ); }

    size_type capacity () const { return capacity_manager().capacity( size() ); }
    bool empty () const { return (storage().begin == storage().end); }
    size_type max_size () const { return allocator().max_size(); }
    void reserve ( size_type capacity ) { reallocate( capacity_manager().reserve( size(), capacity ) ); }
    void resize ( size_type size, const value_type &value = value_type() );
    size_type size () const { return (storage().end - storage().begin); }

    allocator_type get_allocator () const { return allocator(); }

  protected:
    const my_allocator_type &allocator () const { return impl_; }
    my_allocator_type &allocator () { return impl_; }

    const Storage &storage () const { return impl_.storage; }
    Storage &storage () { return impl_.storage; }

    const my_capacity_manager_type &capacity_manager () const { storage(); }
    my_capacity_manager_type &capacity_manager () { return storage(); }

    void allocate ( size_type capacity )
    {
      storage().begin = (capacity != size_type( 0 ) ? allocator().allocate( capacity ) : nullptr);
    }

    void reallocate ( std::pair< size_type, size_type > capacity );

    void deallocate ( size_type capacity )
    {
      if( storage().begin )
        allocator().deallocate( storage().begin, capacity );
    }

    void reinitialize ( size_type desired_size )
    {
      destroy();
      std::pair< size_type, size_type > capacity = capacity_manager().resize( size(), desired_size );
      if( capacity.first != capacity.second )
      {
        deallocate( capacity.first );
        allocate( capacity.second );
      }
      storage().end = storage().begin + desired_size;
    }

    void initialize ( size_type desired_size )
    {
      allocate( capacity_manager().resize( size_type( 0 ), desired_size ).second );
      storage().end = storage().begin + desired_size;
    }

    void construct ( const value_type &value )
    {
      for( pointer it = storage().begin; it != storage().end; ++it )
        allocator().construct( it, value );
    }

    template< class InputIterator >
    void construct ( InputIterator first, InputIterator last )
    {
      for( pointer it = storage().begin; it != storage().end; ++it, ++first )
        allocator().construct( it, *first );
      assert( first == last );
    }

    void destroy ()
    {
      for( pointer it = storage().begin; it != storage().end; ++it )
        allocator().destroy( it );
    }

    void rangeCheck ( size_type i ) const
    {
      if( i >= size() )
        DUNE_THROW( RangeError, "Out of range: i = " << i << " >= " << size() << " = size()." );
    }

  private:
    Impl impl_;
  };



  // Comparison Operators
  // --------------------

  template< class T, class CM, class A >
  inline bool operator== ( const DynamicArray< T, CM, A > &a, const DynamicArray< T, CM, A > &b )
  {
    return (a.size() == b.size()) && std::equal( a.begin(), a.end(), b.begin() );
  }

  template< class T, class CM, class A >
  inline bool operator!= ( const DynamicArray< T, CM, A > &a, const DynamicArray< T, CM, A > &b )
  {
    return !(a == b);
  }



  // Implementation of DynamicArray
  // ------------------------------

  template< class T, class CM, class A >
  inline void DynamicArray< T, CM, A >
  ::reallocate ( std::pair< size_type, size_type > capacity )
  {
    if( capacity.first != capacity.second )
    {
      pointer current_begin = storage().begin;
      pointer current_end = storage().end;

      allocate( capacity.second );

      pointer wit = storage().begin;
      for( pointer rit = current_begin; rit != current_end; ++wit, ++rit )
      {
        allocator().construct( wit, *rit );
        allocator().destory( rit );
      }
      storage().end = wit;

      allocator().destroy( current_begin, capacity.first );
    }
  }


  template< class T, class CM, class A >
  inline void DynamicArray< T, CM, A >
  ::resize ( size_type size, const value_type &value )
  {
    const size_type current_size = (storage().end - storage().begin);
    if( size < current_size )
    {
      for( pointer it = storage().begin + size; it != storage().end; ++it )
        allocator().destroy( it );
      storage().end = storage().begin + size;

      reallocate( capacity_manager.resize( current_size, size ) );
    }
    else if( size > current_size )
    {
      reallocate( capacity_manager.resize( current_size, size ) );

      storage().end = storage().begin + size;
      for( pointer it = storage().begin + current_size; it != storage().end; ++it )
        allocator().construct( it, value );
    }
  }

} // namespace Dune

#endif // #ifndef DUNE_DYNAMICARRAY_HH
