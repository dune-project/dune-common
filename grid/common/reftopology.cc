// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/** \file
 * \brief Implementation of the reference topologies
 */
#include "reftopology.hh"

using namespace Dune;

namespace Dune {

  template <int inputCodim, int outputCodim>
  void Dune::ReferenceTopologySet::getSubEntities(const GeometryType& eType,
                                                  int entity,
                                                  const int*& result,
                                                  int& n)
  {
    DUNE_THROW(NotImplemented, "no subEntity information for these codims");
  }


  template <>
  void Dune::ReferenceTopologySet::getSubEntities<1,1>(const GeometryType& eType,
                                                       int entity,
                                                       const int*& result,
                                                       int& n)
  {

    switch (eType) {

    case line : {
      n = 1;
      result = &lineVerticesPerFace[entity];
      return;
    }

    default :
      DUNE_THROW(NotImplemented, "No subEntity handling for codims (1,2) and element type" << eType);
    }

  }

  template <>
  void Dune::ReferenceTopologySet::getSubEntities<1,2>(const GeometryType& eType,
                                                       int entity,
                                                       const int*& result,
                                                       int& n)
  {

    switch (eType) {

    case line : {
      n = 1;
      result = &lineVerticesPerFace[entity];
      return;
    }

    case triangle : {
      n = 2;
      result = &triangleVerticesPerFace[entity][0];
      return;
    }

    case quadrilateral : {
      n = 2;
      result = &quadrilateralVerticesPerFace[entity][0];
      return;
    }

    default :
      DUNE_THROW(NotImplemented, "No subEntity handling for codims (1,2) and element type" << eType);
    }

  }


  template <>
  void Dune::ReferenceTopologySet::getSubEntities<1,3>(const GeometryType& eType,
                                                       int entity,
                                                       const int*& result,
                                                       int& n)
  {

    switch (eType) {

    case tetrahedron : {
      n = 3;
      result = &tetrahedronVerticesPerFace[entity][0];
      return;
    }

    case pyramid : {
      const int faceSizes[5] = {4, 3, 3, 3, 3};

      n = faceSizes[entity];
      result = &pyramidVerticesPerFace[entity][0];
      return;
    }

    case prism : {
      const int faceSizes[5] = {3, 4, 4, 4, 3};

      n = faceSizes[entity];
      result = &prismVerticesPerFace[entity][0];
      return;
    }

    case hexahedron : {
      n = 4;
      result = &hexahedronVerticesPerFace[entity][0];
      return;

    }

    default :
      DUNE_THROW(NotImplemented, "No subEntity handling for codims (1,3) and element type" << eType);
    }

  }

}  // end namespace Dune

const int Dune::ReferenceTopologySet::lineVerticesPerFace[2] =
{0,1};

const int Dune::ReferenceTopologySet::triangleVerticesPerFace[3][2] =
{ {1,2}, {2,0}, {0,1} };

const int Dune::ReferenceTopologySet::quadrilateralVerticesPerFace[4][2] =
{ {2,0}, {1,3} , {0,1} , {3,2}};

const int Dune::ReferenceTopologySet::tetrahedronVerticesPerFace[4][3] =
{ {1, 3, 2},
  {0, 2, 3},
  {0, 3, 1},
  {0, 1, 2} };

const int Dune::ReferenceTopologySet::pyramidVerticesPerFace[5][4] =
{ {0, 3, 2, 1},
  {0, 1, 4, -1},
  {1, 2, 4, -1},
  {2, 3, 4, -1},
  {0, 4, 3, -1} };

const int Dune::ReferenceTopologySet::prismVerticesPerFace[5][4] =
{ {0, 2, 1, -1},
  {0, 1, 4, 3},
  {1, 2, 5, 4},
  {0, 3, 5, 2},
  {3, 4, 5, -1} };


const int Dune::ReferenceTopologySet::hexahedronVerticesPerFace[6][4] =
{ {0, 2, 4, 6},
  {1, 3, 5, 7},
  {0, 1, 4, 5},
  {2, 3, 6, 7},
  {0, 1, 2, 3},
  {4, 5, 6, 7} };
