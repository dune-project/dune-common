// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <utility>

#include <dune/python/common/common.hh>

#include <dune/common/hybridutilities.hh>
#include <dune/python/common/dynmatrix.hh>
#include <dune/python/common/dynvector.hh>
#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/common/mpihelper.hh>

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

PYBIND11_MODULE( _common, module )
{
  Dune::Python::addToTypeRegistry<double>(Dune::Python::GenerateTypeName("double"));
  Dune::Python::addToTypeRegistry<int>(Dune::Python::GenerateTypeName("int"));
  Dune::Python::addToTypeRegistry<std::size_t>(Dune::Python::GenerateTypeName("std::size_t"));

  Dune::Python::registerFieldVector<double>(module, std::make_integer_sequence<int, 10>());

  Dune::Hybrid::forEach( std::make_integer_sequence< int, 5 >(), [ module ] ( auto rows ) {
      Dune::Hybrid::forEach( std::make_integer_sequence< int, 5 >(), [ module ] ( auto cols ) {
        Dune::Python::registerFieldMatrix< double, decltype(rows)::value, cols >( module );
      } );
    } );

  Dune::Python::registerDynamicVector<double>(module);
  Dune::Python::registerDynamicMatrix<double>(module);

  int argc = 0;
  char **argv = NULL;
  Dune::MPIHelper::instance(argc,argv);
  Dune::Python::registerCollectiveCommunication(module);

  pybind11::enum_< Dune::Python::Reader > reader( module, "reader" );
  reader.value( "dgf", Dune::Python::Reader::dgf );
  reader.value( "dgfString", Dune::Python::Reader::dgfString );
  reader.value( "gmsh", Dune::Python::Reader::gmsh );
  reader.value( "structured", Dune::Python::Reader::structured );

  pybind11::enum_< Dune::Python::VTKDataType > vtkDataType( module, "DataType" );
  vtkDataType.value( "CellData", Dune::Python::VTKDataType::CellData );
  vtkDataType.value( "PointData", Dune::Python::VTKDataType::PointData );
  vtkDataType.value( "CellVector", Dune::Python::VTKDataType::CellVector );
  vtkDataType.value( "PointVector", Dune::Python::VTKDataType::PointVector );

  pybind11::enum_< Dune::PartitionType > partitionType( module, "PartitionType" );
  partitionType.value( "Interior", Dune::InteriorEntity );
  partitionType.value( "Border", Dune::BorderEntity );
  partitionType.value( "Overlap", Dune::OverlapEntity );
  partitionType.value( "Front", Dune::FrontEntity );
  partitionType.value( "Ghost", Dune::GhostEntity );

  pybind11::enum_< Dune::PartitionIteratorType > partitionIteratorType( module, "PartitionIteratorType" );
  partitionIteratorType.value( "Interior", Dune::Interior_Partition );
  partitionIteratorType.value( "InteriorBorder", Dune::InteriorBorder_Partition );
  partitionIteratorType.value( "Overlap", Dune::Overlap_Partition );
  partitionIteratorType.value( "OverlapFront", Dune::OverlapFront_Partition );
  partitionIteratorType.value( "All", Dune::All_Partition );
  partitionIteratorType.value( "Ghost", Dune::Ghost_Partition );

  pybind11::enum_< Dune::InterfaceType > interfaceType( module, "InterfaceType" );
  interfaceType.value( "InteriorBorder_InteriorBorder", Dune::InteriorBorder_InteriorBorder_Interface );
  interfaceType.value( "InteriorBorder_All", Dune::InteriorBorder_All_Interface );
  interfaceType.value( "Overlap_OverlapFront", Dune::Overlap_OverlapFront_Interface );
  interfaceType.value( "Overlap_All", Dune::Overlap_All_Interface );
  interfaceType.value( "All_All", Dune::All_All_Interface );

  pybind11::enum_< Dune::CommunicationDirection > communicationDirection( module, "CommunicationDirection" );
  communicationDirection.value( "Forward", Dune::ForwardCommunication );
  communicationDirection.value( "Backward", Dune::BackwardCommunication );

  pybind11::enum_<Dune::VTK::OutputType> vtkOutputType(module, "OutputType");
  vtkOutputType.value("ascii", Dune::VTK::OutputType::ascii);
  vtkOutputType.value("base64", Dune::VTK::OutputType::base64);
  vtkOutputType.value("appendedraw", Dune::VTK::OutputType::appendedraw);
  vtkOutputType.value("appendedbase64", Dune::VTK::OutputType::appendedbase64);

  pybind11::enum_< Dune::Python::Marker > marker( module, "Marker" );
  marker.value( "coarsen", Dune::Python::Marker::Coarsen );
  marker.value( "keep", Dune::Python::Marker::Keep );
  marker.value( "refine", Dune::Python::Marker::Refine );
}
