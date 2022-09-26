// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_NUMPYVECTOR_HH
#define DUNE_PYTHON_COMMON_NUMPYVECTOR_HH

#include <dune/common/exceptions.hh>
#include <dune/common/densevector.hh>
#include <dune/common/ftraits.hh>

#include <dune/python/pybind11/numpy.h>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

namespace Dune
{

  namespace Python
  {

    // Internal Forward Declarations
    // -----------------------------

    template< class T >
    class NumPyVector;

  } // namespace Python



  // DenseMatVecTraits for NumPyVector
  // ---------------------------------

  template< class T >
  struct DenseMatVecTraits< Python::NumPyVector< T > >
  {
    typedef Python::NumPyVector< T > derived_type;
    typedef pybind11::array_t< T > container_type;
    typedef T value_type;
    typedef std::size_t size_type;
  };



  // FieldTraits for NumPyVector
  // ---------------------------

  template< class T >
  struct FieldTraits< Python::NumPyVector< T > >
  {
    typedef typename FieldTraits< T >::field_type field_type;
    typedef typename FieldTraits< T >::real_type real_type;
  };


  namespace Python
  {

    template< class T >
    class NumPyVector
      : public DenseVector< NumPyVector< T > >
    {
      typedef NumPyVector< T > This;
      typedef DenseVector< NumPyVector< T > > Base;

    public:
      typedef typename Base::size_type size_type;
      typedef typename Base::value_type value_type;

      explicit NumPyVector ( size_type size )
        : array_( pybind11::buffer_info( nullptr, sizeof( T ),
                  pybind11::format_descriptor< T >::value, 1, { size }, { sizeof( T ) } )
                ),
          dataPtr_( static_cast< value_type * >( array_.request(true).ptr ) ),
          size_(size)
      {}

      NumPyVector ( pybind11::buffer buf )
        : array_( buf ),
          dataPtr_( nullptr ),
          size_( 0 )
      {
        pybind11::buffer_info info = buf.request();
        if (info.ndim != 1)
          DUNE_THROW( InvalidStateException, "NumPyVector can only be created from one-dimensional array" );
        size_ = info.shape[0];

        dataPtr_ = static_cast< value_type * >( array_.request(true).ptr );
      }

      NumPyVector ( const This &other ) = delete;
      NumPyVector ( This &&other ) = delete;

      ~NumPyVector() {}

      This &operator= ( const This &other ) = delete;
      This &operator= ( This &&other ) = delete;

      operator pybind11::array_t< T > () const { return array_; }

      const value_type &operator[] ( size_type index ) const
      {
        return data()[ index ];
      }
      value_type &operator[] ( size_type index )
      {
        return data()[ index ];
      }
      value_type &vec_access ( size_type index )
      {
        return data()[ index ];
      }
      const value_type &vec_access ( size_type index ) const
      {
        return data()[ index ];
      }

      inline const value_type *data () const
      {
        assert( dataPtr_ );
        return dataPtr_;
      }
      inline value_type *data ()
      {
        assert( dataPtr_ );
        return dataPtr_;
      }
      pybind11::array_t< T > &coefficients()
      {
        return array_;
      }
      pybind11::array_t< T > &coefficients() const
      {
        return array_;
      }

      size_type size () const
      {
        return size_;
      }
      size_type vec_size () const
      {
        return size_;
      }

    protected:
      pybind11::array_t< T > array_;
      value_type* dataPtr_;
      size_type size_;
    };

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_NUMPYVECTOR_HH
