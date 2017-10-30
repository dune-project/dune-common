#ifndef DUNE_COMMON_STD_OPTIONAL_HH
#define DUNE_COMMON_STD_OPTIONAL_HH

#include <cassert>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef DUNE_HAVE_CXX_OPTIONAL
#include <optional>
#endif // #ifdef DUNE_HAVE_CXX_OPTIONAL


namespace Dune
{

  namespace Std
  {

#ifdef DUNE_HAVE_CXX_OPTIONAL
    // In case of C++ standard >= 17 we forward optionals into our namespace
    template< class T >
    using optional = std::optional< T >;
#else
    // In case of C++ standard < 17 we take the fallback implementation

    // nullopt
    // -------

    struct nullopt_t {};

    namespace
    {

      const nullopt_t nullopt = {};

    } // anonymous namespace



    // in_place
    // --------

    struct in_place_t {};

    namespace
    {

      const in_place_t in_place =  {};

    } // anonymous namespace




    // bad_optional_access
    // -------------------

    class bad_optional_access
      : public std::logic_error
    {
    public:
      explicit bad_optional_access ( const std::string &what ) : std::logic_error( what ) {}
      explicit bad_optional_access ( const char *what ) : std::logic_error( what ) {}
    };




    // optional
    // --------

    /**
     * \class optional
     */
    template< class T >
    class optional
    {
    public:
      /** \brief type of value */
      typedef T value_type;

      /**
       * \name Construction
       * \{
       */

      constexpr optional () noexcept : engaged_( false ) {}

      constexpr optional ( nullopt_t ) noexcept : engaged_( false ) {}

      template< class U = value_type,
                std::enable_if_t< std::is_constructible< value_type, U&& >::value, int > = 0,
                std::enable_if_t< !std::is_convertible< U&&, value_type >::value, int > = 0 >
      explicit constexpr optional ( U && value )
        : engaged_( true ), value_( std::forward< U >( value ) )
      {}

      template< class U = value_type,
                std::enable_if_t< std::is_constructible< value_type, U&& >::value, int > = 0,
                std::enable_if_t< std::is_convertible< U&&, value_type >::value, int > = 0 >
      constexpr optional ( U && value )
        : engaged_( true ), value_( std::forward< U >( value ) )
      {}

      optional ( const value_type &value ) : engaged_( true ), value_( value ) {}
      optional ( value_type &&value ) : engaged_( true ), value_( std::move( value ) ) {}

      template< class... Args >
      explicit constexpr optional ( in_place_t, Args &&... args )
        : engaged_( true ), value_( std::forward< Args >( args )... )
      {}

      /** \} */

      /**
       * \name Copying and Assignment
       * \{
       */

      optional ( const optional &other ) noexcept( std::is_nothrow_copy_constructible< T >::value )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( other.value_ );
      }

      optional ( optional &&other ) noexcept( std::is_nothrow_move_constructible< T >::value )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( std::move( other.value_ ) );
      }

      template< class U,
                std::enable_if_t< std::is_constructible< value_type, const U& >::value, int > = 0,
                std::enable_if_t< !std::is_constructible< value_type, optional< U >& >::value, int > = 0,
                std::enable_if_t< !std::is_constructible< value_type, const optional< U >& >::value, int > = 0,
                std::enable_if_t< !std::is_constructible< value_type, optional< U >&& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< const optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&&, value_type >::value, int > = 0,
                std::enable_if_t< !std::is_convertible< const U&, value_type >::value, int > = 0 >
      explicit optional ( const optional< U > &other )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( other.value_ );
      }

      template< class U,
                std::enable_if_t< std::is_constructible< value_type, const U& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, const optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >&& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< const optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&&, value_type >::value, int > = 0,
                std::enable_if_t< std::is_convertible< const U&, value_type >::value, int > = 0 >
      optional ( const optional< U > &other )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( other.value_ );
      }

      template< class U,
                std::enable_if_t< std::is_constructible< value_type, const U& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, const optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >&& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< const optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&&, value_type >::value, int > = 0,
                std::enable_if_t< !std::is_convertible< const U&, value_type >::value, int > = 0 >
      explicit optional ( optional< U > &&other )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( std::move( other.value_ ) );
      }

      template< class U,
                std::enable_if_t< std::is_constructible< value_type, const U& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, const optional< U >& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< value_type, optional< U >&& >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< const optional< U >&, value_type >::value, int > = 0,
                                std::enable_if_t< !std::is_constructible< optional< U >&&, value_type >::value, int > = 0,
                std::enable_if_t< std::is_convertible< const U&, value_type >::value, int > = 0 >
      optional ( optional< U > &&other )
        : engaged_( other.engaged_ )
      {
        if( engaged_ )
          new( &value_ ) value_type( std::move( other.value_ ) );
      }

      optional &operator= ( nullopt_t ) noexcept
      {
        if( engaged_ )
          value_.~value_type();
        engaged_ = false;
        return *this;
      }

      optional &operator= ( const optional &other ) noexcept( std::is_nothrow_copy_constructible< T >::value && std::is_nothrow_copy_assignable< T >::value )
      {
        if( engaged_ )
        {
          if( other.engaged_ )
            value_ = other.value_;
          else
            value_.~value_type();
        }
        else if( other.engaged_ )
          new( &value_ ) value_type( other.value_ );
        engaged_ = other.engaged_;
        return *this;
      }

      optional &operator= ( optional &&other ) noexcept( std::is_nothrow_move_constructible< T >::value && std::is_nothrow_move_assignable< T >::value )
      {
        if( engaged_ )
        {
          if( other.engaged_ )
            value_ = std::move( other.value_ );
          else
            value_.~value_type();
        }
        else if( other.engaged_ )
          new( &value_ ) value_type( std::move( other.value_ ) );
        engaged_ = other.engaged_;
        return *this;
      }

      template< class U = value_type >
      typename std::enable_if< std::is_constructible< value_type, U >::value && std::is_assignable< value_type, U >::value, optional & >::type
      operator= ( U &&value )
      {
        if( engaged_ )
          value_ = std::move( value );
        else
          new( &value_ ) value_type( std::forward< U >( value ) );
        engaged_ = true;
        return *this;
      }

      /** \} */

      ~optional ()
      {
        if( engaged_ )
          value_.~value_type();
      }

      /**
       * \name Observers
       * \{
       */

      /** \brief return \b true if optional is engaged, \b false otherwise */
      explicit constexpr operator bool () const noexcept { return engaged_; }

      /** \brief dereference pointer */
      const value_type &operator* () const noexcept { assert( engaged_ ); return value_; }
      /** \brief dereference pointer */
      value_type &operator* () noexcept { assert( engaged_ ); return value_; }

      /** \brief pointer operator */
      const value_type *operator->() const noexcept { assert( engaged_ ); return &value_; }
      /** \brief pointer operator */
      value_type *operator->() noexcept { assert( engaged_ ); return &value_; }

      const value_type &value () const
      {
        if( engaged_ )
          return value_;
        else
          throw bad_optional_access( "Cannot access value of disengaged optional." );
      }

      value_type &value ()
      {
        if( engaged_ )
          return value_;
        else
          throw bad_optional_access( "Cannot access value of disengaged optional." );
      }

      template< class U >
      value_type value_or ( U &&value ) const
      {
        return (engaged_ ? value_ : static_cast< value_type >( std::forward< U >( value ) ));
      }

      /** \} */

      /**
       * \name Modifiers
       * \{
       */

      template< class... Args >
      void emplace ( Args &&... args )
      {
        *this = nullopt;
        // note: At this point, the optional is disengaged. If the following
        //       constructor throws, the object is left in a disengaged state.
        new( &value_ ) value_type( std::forward< Args >( args )... );
        engaged_ = true;
      }

      void reset () noexcept
      {
        if( engaged_)
        {
          value_.~value_type();
          engaged_ = false;
        }
      }

      void swap ( optional &other ) noexcept( std::is_nothrow_move_constructible< T >::value && noexcept( std::swap( std::declval< T & >(), std::declval< T & >() ) ) )
      {
        std::swap( engaged_, other.engaged_ );
        if( engaged_)
        {
          if( other.engaged_ )
            std::swap( value_, other.value_ );
          else
          {
            new( &value_ ) value_type( std::move( other.value_ ) );
            other.value_.~value_type();
          }
        }
        else if( other.engaged_ )
        {
          new( &other.value_ ) value_type( std::move( value_ ) );
          value_.~value_type();
        }
      }

      /** \} */

    private:
      bool engaged_;
      union { value_type value_; };
    };



    // Relatonal Operators for optional
    // --------------------------------

    template< class T >
    inline static constexpr bool operator== ( const optional< T > &lhs, const optional< T > &rhs )
    {
      return (lhs && rhs ? *lhs == *rhs : static_cast< bool >( lhs ) == static_cast< bool >( rhs ));
    }


    template< class T >
    inline static constexpr bool operator< ( const optional< T > &lhs, const optional< T > &rhs )
    {
      return (rhs && (lhs ? std::less< T >()( *lhs, *rhs ) : true));
    }


    template< class T >
    inline static constexpr bool operator== ( const optional< T > &lhs, nullopt_t ) noexcept
    {
      return !lhs;
    }

    template< class T >
    inline static constexpr bool operator== ( nullopt_t, const optional< T > &rhs ) noexcept
    {
      return !rhs;
    }

    template< class T >
    inline static constexpr bool operator< ( const optional< T > &lhs, nullopt_t ) noexcept
    {
      return false;
    }

    template< class T >
    inline static constexpr bool operator< ( nullopt_t, const optional< T > &rhs ) noexcept
    {
      return static_cast< bool >( rhs );
    }

    template< class T >
    inline static constexpr bool operator== ( const optional< T > &lhs, const T &rhs )
    {
      return (lhs && (*lhs == rhs));
    }

    template< class T >
    inline static constexpr bool operator== ( const T &lhs, const optional< T > &rhs )
    {
      return (rhs && (lhs == *rhs));
    }

    template< class T >
    inline static constexpr bool operator< ( const optional< T > &lhs, const T &rhs )
    {
      return (lhs ? std::less< T >()( *lhs, rhs ) : true);
    }

    template< class T >
    inline static constexpr bool operator< ( const T &lhs, const optional< T > &rhs )
    {
      return (rhs ? std::less< T >()( lhs, *rhs ) : false);
    }



    // make_optional
    // -------------

    template< class T >
    inline static constexpr optional< typename std::decay< T >::type > make_optional ( T &&value )
    {
      return optional< typename std::decay< T >::type >( std::forward< T >( value ) );
    }

#endif //#ifdef DUNE_HAVE_CXX_OPTIONAL

  } // namespace Std

} // namespace Dune


#ifndef DUNE_HAVE_CXX_OPTIONAL
namespace std
{

  // swap for optional
  // -----------------

  template< class T >
  inline static void swap ( Dune::Std::optional< T > &lhs, Dune::Std::optional< T > &rhs ) noexcept( noexcept( lhs.swap( rhs ) ) )
  {
    lhs.swap( rhs );
  }



  // hash for optional
  // -----------------

  template< class T >
  struct hash< Dune::Std::optional< T > >
  {
    std::size_t operator() ( const Dune::Std::optional< T > &arg ) const
    {
      return (arg ? std::hash< T >()( arg ) : 0);
    }
  };

} // namespace std

#endif //#ifndef DUNE_HAVE_CXX_OPTIONAL

#endif // #ifndef DUNE_COMMON_STD_OPTIONAL_HH
