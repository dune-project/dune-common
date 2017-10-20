#ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
#define DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH

#include <cassert>
#include <cstddef>

#include <type_traits>
#include <utility>

#include <dune/geometry/type.hh>

#include <dune/grid/common/datahandleif.hh>

#include <dune/python/pybind11/numpy.h>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< class, template< int > class >
  class MultipleCodimMultipleGeomTypeMapper;



  namespace Python
  {

    // NumPyCommDataHandle
    // -------------------

    template< class Mapper, class T, class Function >
    class NumPyCommDataHandle;

    template< class GV, template< int > class Layout, class T, class Function >
    class NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV, Layout >, T, Function >
      : public CommDataHandleIF< NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV, Layout >, T, Function >, T >
    {
      typedef NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV, Layout >, Vector, Function > This;

      typedef MultipleCodimMultipleGeomTypeMapper< GV, Layout > Mapper;

    public:
      NumPyCommDataHandle ( const Mapper &mapper, pybind11::array_t< T > array, Function function = Function() )
        : mapper_( mapper ), buffer_( array.request() ), function_( function )
      {
        if( buffer_.strides[ 0 ] * buffer_.shape[ 0 ] != buffer_.size )
          DUNE_THROW( Exception, "NumPyCommDataHandle requires contiguous array entries." );
      }

      bool contains ( int dim, int codim ) const
      {
        // mapper_.layout is private, so assume it is default-constructed
        Layout< GV::dimension > layout;

        // a codim is contained, if the layout contains any geometry type of
        // dimension dim - codim
        const int mydim = dim - codim;
        for( unsigned int id = 0u; id < (1u << mydim); id += 2u )
        {
          if( layout.contains( GeometryType( id, mydim ) ) )
            return true;
        }
        return layout.contains( GeometryType( GeometryType::none, mydim ) );
      }

      bool fixedsize ( int dim, int codim ) const
      {
        // if a codim is not contained, it has fixed size (namely zero)
        if( !contains( dim, codim ) )
          return true;

        // mapper_.layout is private, so assume it is default-constructed
        Layout< GV::dimension > layout;

        // a codim has fixed size, if the layout contains all geometry types of
        // dimension dim - codim
        const int mydim = dim - codim;
        for( unsigned int id = 0u; id < (1u << mydim); id += 2u )
        {
          if( !layout.contains( GeometryType( id, mydim ) ) )
            return false;
        }
        return layout.contains( GeometryType( GeometryType::none, mydim ) );
      }

      template< class Entity >
      std::size_t size ( const Entity &entity ) const
      {
        typename Mapper::Index index;
        return (mapper_.contains( entity, index ) ? buffer_.strides[ 0 ] : 0);
      }

      template< class Buffer, class Entity >
      void gather ( Buffer &buffer, const Entity &entity ) const
      {
        typename Mapper::Index index;
        if( !mapper_.contains( entity, index ) )
          return;

        for( std::size_t i = 0; i < buffer_.strides[ 0 ]; ++i )
          buffer.write( static_cast< T * >( buffer_.ptr )[ index*buffer_.strides[ 0 ] + i ] );
      }

      template< class Buffer, class Entity >
      void scatter ( Buffer &buffer, const Entity &entity, std::size_t n )
      {
        assert( n == size( entity ) );
        typename Mapper::Index index;
        if( !mapper_.contains( entity, index ) )
          return;

        for( std::size_t i = 0; i < buffer_.strides[ 0 ]; ++i )
        {
          T remote;
          buffer.read( remote );
          T &local = static_cast< T * >( buffer_.ptr )[ index*buffer_.strides[ 0 ] + i ];
          local = function_( local, remote );
        }
      }

    private:
      const Mapper &mapper_;
      pybind11::buffer_info buffer_;
      Function function_;
    };



    // vectorCommDataHandle
    // --------------------

    template< class Mapper, class T, class Function >
    inline static NumPyCommDataHandle< Mapper, T, Function >
    numPyCommDataHandle( const Mapper &mapper, pybind11::array_t< T > array, Function function )
    {
      return NumPyCommDataHandle< Mapper, Vector, Function >( mapper, std::move( array ), std::move( function ) );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
