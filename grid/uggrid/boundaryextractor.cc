// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/common/exceptions.hh>

#include "boundaryextractor.hh"

void Dune::BoundaryExtractor::detectBoundarySegments(const std::vector<unsigned char>& elementTypes,
                                                     const std::vector<unsigned int>& elementVertices,
                                                     std::set<FieldVector<int, 2>, CompareBoundarySegments<2> >& boundarySegments)
{
  // The vertices that form the edges of a triangle -- in UG numbering
  static const int triIdx[][2] = {
    {0,1},{1,2},{2,0}
  };

  // The vertices that form the edges of a quadrilateral -- in UG numbering
  static const int quadIdx[][2] = {
    {0,1},{1,2},{2,3},{2,0}
  };

  boundarySegments.clear();
  unsigned int currentBase = 0;

  for (size_t i=0; i<elementTypes.size(); i++) {

    int verticesPerElement = elementTypes[i];

    for (int k=0; k<verticesPerElement; k++) {

      FieldVector<int, 2> v;
      if (verticesPerElement==3) {
        v[0] = elementVertices[currentBase+triIdx[k][0]];
        v[1] = elementVertices[currentBase+triIdx[k][1]];
      } else {
        v[0] = elementVertices[currentBase+quadIdx[k][0]];
        v[1] = elementVertices[currentBase+quadIdx[k][1]];
      }

      // Check if new face exists already in the list
      // (then it is no boundary face)
      std::pair<std::set<FieldVector<int, 2>, CompareBoundarySegments<2> >::iterator,bool> status = boundarySegments.insert(v);
      if (!status.second)         //  Not inserted because already existing
        boundarySegments.erase(status.first);

    }

    currentBase += verticesPerElement;

  }

}

/** \todo This is quadratic --> very slow */
void Dune::BoundaryExtractor::detectBoundarySegments(const std::vector<unsigned char>& elementTypes,
                                                     const std::vector<unsigned int>& elementVertices,
                                                     std::set<FieldVector<int, 4>, CompareBoundarySegments<3> >& boundarySegments)
{
  int numElements = elementTypes.size();

  // The vertices that form the faces of a tetrahedron -- in UG numbering
  // Double numbers mean the face is actually a triangle
  static const int tetraIdx[][4] = {
    {1,3,2,2},{0,2,3,3},{0,3,1,1},{0,1,2,2}
  };

  // The vertices that form the faces of a pyramid -- in UG numbering
  static const int pyramidIdx[][4] = {
    {0,1,2,3},{0,4,1,1},{1,4,2,2},{3,2,4,4},{0,3,4,4}
  };

  // The vertices that form the faces of a prism -- in UG numbering
  static const int prismIdx[][4] = {
    {0,1,2,2},{0,3,4,1},{1,4,5,2},{0,2,5,3},{3,5,4,4}
  };

  // The vertices that form the faces of a hexahedron -- in UG numbering
  static const int hexaIdx[][4] = {
    {0,4,5,1},{1,5,6,2},{2,6,7,3},{3,7,4,0},{4,7,6,5},{1,2,3,0}
  };

  // Number of faces for tetrahedra, pyramids, prisms, hexahedra
  // The zeros are just fill-in.
  static const int numFaces[9] = {0,0,0,0,4,5,5,0,6};
  boundarySegments.clear();

  // An index into the list of element vertices pointing to the current element
  int currentElement = 0;

  for (int i=0; i<numElements; i++) {

    for (int k=0; k<numFaces[elementTypes[i]]; k++) {

      FieldVector<int, 4> v;

      switch (elementTypes[i]) {
      case 4 :       // tetrahedron
        for (int j=0; j<4; j++)
          v[j] = elementVertices[currentElement+tetraIdx[k][j]];
        break;
      case 5 :       // pyramid
        for (int j=0; j<4; j++)
          v[j] = elementVertices[currentElement+pyramidIdx[k][j]];
        break;
      case 6 :       // prism
        for (int j=0; j<4; j++)
          v[j] = elementVertices[currentElement+prismIdx[k][j]];
        break;
      case 8 :       // hexahedron
        for (int j=0; j<4; j++)
          v[j] = elementVertices[currentElement+hexaIdx[k][j]];
        break;
      default :
        DUNE_THROW(Exception, "Can't handle elements with " << elementTypes[i] << " vertices!");
      }

      // Check whether the faces is degenerated to a triangle
      if (v[2]==v[3])
        v[3] = -1;

      // Check if new face exists already in the list
      // (then it is no boundary face)

      std::pair<std::set<FieldVector<int, 4>, CompareBoundarySegments<3> >::iterator,bool> status = boundarySegments.insert(v);
      if (!status.second)         //  Not inserted because already existing
        boundarySegments.erase(status.first);

    }

    currentElement += elementTypes[i];

  }

}

template<int NUM_VERTICES>
int Dune::BoundaryExtractor::detectBoundaryNodes(const std::set< Dune::FieldVector<int, NUM_VERTICES>, CompareBoundarySegments<(NUM_VERTICES+2)/2> >& boundarySegments,
                                                 int noOfNodes,
                                                 std::vector<int>& isBoundaryNode)
{
  isBoundaryNode.resize(noOfNodes);

  int UGNodeIdxCounter = 0;

  for (int i=0; i<noOfNodes; i++)
    isBoundaryNode[i] = -1;

  typename std::set< Dune::FieldVector<int, NUM_VERTICES>, CompareBoundarySegments<(NUM_VERTICES+2)/2> >::iterator it = boundarySegments.begin();

  for (; it!=boundarySegments.end(); ++it) {

    for (int j=0; j<NUM_VERTICES; j++)
      if ((*it)[j]!=-1 && isBoundaryNode[(*it)[j]] == -1)
        isBoundaryNode[(*it)[j]] = 1;

  }

  for (unsigned int i=0; i<isBoundaryNode.size(); i++)
    if (isBoundaryNode[i] != -1)
      isBoundaryNode[i] = UGNodeIdxCounter++;

  return UGNodeIdxCounter;
}


// //////////////////////////////////////////////////////////////////////////////
//   Explicitly instantiate the member templates that we need.  We need
//   2 and 4, because this is the maximum number of vertices a boundary segment
//   can have in 2d and 3d, respectively.
// //////////////////////////////////////////////////////////////////////////////

template int Dune::BoundaryExtractor::detectBoundaryNodes<2>(const std::set<FieldVector<int, 2>, CompareBoundarySegments<2> >& boundarySegments,
                                                             int noOfNodes,
                                                             std::vector<int>& isBoundaryNode);

template int Dune::BoundaryExtractor::detectBoundaryNodes<4>(const std::set<FieldVector<int, 4>, CompareBoundarySegments<3> >& boundarySegments,
                                                             int noOfNodes,
                                                             std::vector<int>& isBoundaryNode);
