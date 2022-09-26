// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_PYTHONVECTOR_HH
#define DUNE_PYTHON_COMMON_PYTHONVECTOR_HH

#include <cstddef>

#include <dune/common/densevector.hh>

#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace Python
  {

    // Internal Forward Declarations
    // -----------------------------

    template< class K >
    class PythonVector;

  } // namespace Python



  // DenseMatVecTraits for PythonVector
  // ----------------------------------

  template< class K >
  struct DenseMatVecTraits< Python::PythonVector< K > >
  {
    typedef Python::PythonVector< K > derived_type;
    typedef K value_type;
    typedef std::size_t size_type;
  };



  // FieldTraits for PythonVector
  // ----------------------------

  template< class K >
  struct FieldTraits< Python::PythonVector< K > >
  {
    typedef typename FieldTraits< K >::field_type field_type;
    typedef typename FieldTraits< K >::real_type real_type;
  };



  namespace Python
  {

    template< class K >
    class PythonVector
      : public Dune::DenseVector< PythonVector< K > >
    {
      typedef PythonVector< K > This;
      typedef Dune::DenseVector< PythonVector< K > > Base;

    public:
      typedef typename Base::size_type size_type;
      typedef typename Base::field_type field_type;

      explicit PythonVector ( pybind11::buffer buffer )
        : buffer_( buffer ), info_( buffer_.request() )
      {
        if( info_.format != pybind11::format_descriptor< field_type >::format() )
          throw std::runtime_error( "Incompatible buffer format." );
        if( info_.ndim != 1 )
          throw std::runtime_error( "PythonVector can only be instantiated from one-dimensional buffers." );
        stride_ = info_.strides[ 0 ] / sizeof( field_type );
      }

      PythonVector ( const This & ) = delete;
      PythonVector ( This && ) = default;

      This &operator= ( const This & ) = delete;
      This &operator= ( This && ) = default;

      const field_type &operator[] ( size_type i ) const
      {
        return static_cast< const field_type * >( info_.ptr )[ i*stride_ ];
      }

      field_type &operator[] ( size_type i )
      {
        return static_cast< field_type * >( info_.ptr )[ i*stride_ ];
      }

      size_type size () const { return info_.shape[ 0 ]; }

    private:
      pybind11::buffer buffer_;
      pybind11::buffer_info info_;
      size_type stride_;
    };

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_PYTHONVECTOR_HH
