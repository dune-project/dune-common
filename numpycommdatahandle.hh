#ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
#define DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH

#include <cassert>
#include <cstddef>

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
      NumPyCommDataHandle ( const Mapper &mapper, std::vector<pybind11::array_t< T >> arrays, Function function = Function() )
        : mapper_( mapper ), buffers_(arrays.size()), function_( function )
      {
        for ( size_t i=0;i<arrays.size();++i)
        {
          buffers_[i] = arrays[i].request();
          if( buffers_[i].strides[ 0 ] * buffers_[i].shape[ 0 ] != buffers_[i].size )
            DUNE_THROW( Exception, "NumPyCommDataHandle requires contiguous array entries." );
          if( buffers_[i].size/sizeof(double) == mapper.size() )
            DUNE_THROW( Exception, "Array does not match mapper in construction of NumPyCommDataHandle." );
          if( buffers_[i].strides[ 0 ] == buffers_[0].strides[ 0 ] )
            DUNE_THROW( Exception, "Arrays passed to NumPyCommDataHandle do not match." );
        }
      }
      NumPyCommDataHandle ( const Mapper &mapper, pybind11::array_t< T > array, Function function = Function() )
        : NumPyCommDataHandle( mapper, std::vector<pybind11::array_t<T>>{array}, function )
      {}

      bool contains ( int dim, int codim ) const
      {
        for (const auto &gt : mapper_.types( codim ) )
          if ( mapper_.size(gt)>0 )
            return true;
        return false;
      }

      bool fixedsize ( int dim, int codim ) const
      {
        size_t size = 0;
        for (const auto &gt : mapper_.types( codim ) )
          if ( mapper_.size(gt) != size )
            return false;
        return true;
      }

      template< class Entity >
      std::size_t size ( const Entity &entity ) const
      {
        auto size = mapper_.size( entity.type() );
        return buffers_[0].strides[ 0 ]/sizeof(T) * size;
      }

      template< class Buffer, class Entity >
      void gather ( Buffer &commBuffer, const Entity &entity ) const
      {
        for( const auto index : mapper_.indices( entity ) )
          for( const auto &buffer : buffers_ )
            for( std::size_t r = 0; r < buffer.strides[ 0 ]/sizeof(double); ++r )
              commBuffer.write( static_cast< T * >( buffer.ptr )[ index*buffer.strides[ 0 ]/sizeof(double) + r ] );
      }

      template< class Buffer, class Entity >
      void scatter ( Buffer &commBuffer, const Entity &entity, std::size_t n )
      {
        assert( n == size( entity ) );

        for( const auto index : mapper_.indices( entity ) )
          for( auto &buffer : buffers_ )
            for( std::size_t r = 0; r < buffer.strides[ 0 ]/sizeof(double); ++r )
            {
              T remote;
              commBuffer.read( remote );
              T &local = static_cast< T * >( buffer.ptr )[ index*buffer.strides[ 0 ]/sizeof(double) + r ];
              local = function_( local, remote );
            }
      }

    private:
      const Mapper &mapper_;
      std::vector<pybind11::buffer_info> buffers_;
      Function function_;
    };



    // vectorCommDataHandle
    // --------------------

    template< class Mapper, class T, class Function >
    inline static NumPyCommDataHandle< Mapper, T, Function >
    numPyCommDataHandle( const Mapper &mapper, pybind11::array_t< T > array, Function function )
    {
      return NumPyCommDataHandle< Mapper, T, Function >( mapper, std::move( array ), std::move( function ) );
    }

    template< class Mapper, class T >
    void registerDataHandle( pybind11::handle module, pybind11::class_<NumPyCommDataHandle< Mapper, T, std::function< T( T,T ) >>> cls)
    {
      cls.def( pybind11::init([](Mapper &mapper, pybind11::array_t<double> array, std::function< T( T,T ) > function) {
        return NumPyCommDataHandle<Mapper,T, std::function< T( T,T )>> ( mapper, array, function );
      } ), pybind11::keep_alive<1,2>() );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_UTILITY_VECTORCOMMDATAHANDLE_HH
