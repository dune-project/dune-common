#ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
#define DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <type_traits>
#include <utility>

#include <dune/common/visibility.hh>

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

    template< class GV, class T, class Function >
    class DUNE_PRIVATE NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV >, T, Function >
      : public CommDataHandleIF< NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV >, T, Function >, T >
    {
      typedef NumPyCommDataHandle< MultipleCodimMultipleGeomTypeMapper< GV >, T, Function > This;

      typedef MultipleCodimMultipleGeomTypeMapper< GV > Mapper;

    public:
      NumPyCommDataHandle ( const Mapper &mapper, std::vector< pybind11::array_t< T > > arrays, Function function = Function() )
        : mapper_( mapper ), buffers_( arrays.size() ), function_( function )
      {
        std::transform( arrays.begin(), arrays.end(), buffers_.begin(), [] ( pybind11::array_t< T > &a ) { return a.request(); } );
        itemSize_ = 0;
        for( const pybind11::buffer_info &buffer : buffers_ )
        {
          if( static_cast< std::size_t >( buffer.shape[ 0 ] ) != mapper_.size() )
            pybind11::value_error( "Array does not match mapper in construction of NumPyCommDataHandle." );
          itemSize_ += std::accumulate( buffer.shape.begin()+1, buffer.shape.end(), std::size_t( 1 ), std::multiplies< std::size_t >() );
        }
      }

      NumPyCommDataHandle ( const Mapper &mapper, pybind11::array_t< T > array, Function function = Function() )
        : NumPyCommDataHandle( mapper, std::vector< pybind11::array_t< T > >{ array }, function )
      {}

      bool contains ( int dim, int codim ) const
      {
        const auto &types = mapper_.types( codim );
        return std::any_of( types.begin(), types.end(), [ this ] ( GeometryType gt ) { return mapper_.size( gt ) > 0; } );
      }

      bool fixedsize ( int dim, int codim ) const
      {
        const auto &types = mapper_.types( codim );
        return (std::adjacent_find( types.begin(), types.end(), [ this ] ( GeometryType a, GeometryType b ) { return mapper_.size( a ) != mapper_.size( b ); } ) == types.end());
      }

      template< class Entity >
      std::size_t size ( const Entity &entity ) const
      {
        return mapper_.size( entity.type() ) * itemSize_;
      }

      template< class CommBuffer, class Entity >
      void gather ( CommBuffer &commBuffer, const Entity &entity ) const
      {
        for( const pybind11::buffer_info &buffer : buffers_ )
          for( const auto index : mapper_.indices( entity ) )
            gather( commBuffer, buffer, 1, index*buffer.strides[ 0 ] );
      }

      template< class CommBuffer, class Entity >
      void scatter ( CommBuffer &commBuffer, const Entity &entity, std::size_t n )
      {
        assert( n == size( entity ) );
        for( const pybind11::buffer_info &buffer : buffers_ )
          for( const auto index : mapper_.indices( entity ) )
            scatter( commBuffer, buffer, 1, index*buffer.strides[ 0 ] );
      }

    private:
      template< class CommBuffer >
      void gather ( CommBuffer &commBuffer, const pybind11::buffer_info &buffer, ssize_t dim, ssize_t pos ) const
      {
        if( dim < buffer.ndim )
        {
          for( ssize_t i = 0; i < buffer.shape[ dim ]; ++i )
            gather( commBuffer, buffer, dim+1, pos + i*buffer.strides[ dim ] );
        }
        else
          commBuffer.write( *reinterpret_cast< T * >( static_cast< char * >( buffer.ptr ) + pos ) );
      }

      template< class CommBuffer >
      void scatter ( CommBuffer &commBuffer, const pybind11::buffer_info &buffer, ssize_t dim, ssize_t pos )
      {
        if( dim < buffer.ndim )
        {
          for( ssize_t i = 0; i < buffer.shape[ dim ]; ++i )
            scatter( commBuffer, buffer, dim+1, pos + i*buffer.strides[ dim ] );
        }
        else
        {
          T &local = *reinterpret_cast< T * >( static_cast< char * >( buffer.ptr ) + pos );
          T remote;
          commBuffer.read( remote );
          local = function_( local, remote );
        }
      }

      const Mapper &mapper_;
      std::vector< pybind11::buffer_info > buffers_;
      std::size_t itemSize_;
      Function function_;
    };



    // vectorCommDataHandle
    // --------------------

    template< class Mapper, class T, class Function >
    inline static NumPyCommDataHandle< Mapper, T, Function >
    numPyCommDataHandle ( const Mapper &mapper, pybind11::array_t< T > array, Function function )
    {
      return NumPyCommDataHandle< Mapper, T, Function >( mapper, std::move( array ), std::move( function ) );
    }

    template< class Mapper, class T, class Function, class... options >
    void registerDataHandle ( pybind11::handle module, pybind11::class_< NumPyCommDataHandle< Mapper, T, Function >, options... > cls )
    {
      cls.def( pybind11::init( [] ( Mapper &mapper, pybind11::array_t< T > array, Function function ) {
        return NumPyCommDataHandle< Mapper, T, Function >( mapper, array, function );
      } ), pybind11::keep_alive< 1, 2 >() );
      cls.def( pybind11::init( [] ( Mapper &mapper, std::vector< pybind11::array_t< T > > arrays, Function function ) {
        return NumPyCommDataHandle< Mapper, T, Function >( mapper, arrays, function );
      } ), pybind11::keep_alive< 1, 2 >() );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
