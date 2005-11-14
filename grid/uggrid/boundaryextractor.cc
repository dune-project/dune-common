// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include "boundaryextractor.hh"


/** \todo This is quadratic --> very slow */
void Dune::BoundaryExtractor::detectBoundarySegments(int* elemData,
                                                     int numElems,
                                                     std::vector<FieldVector<int, 2> >& faceList,
                                                     bool containsOnlyTriangles)
{
  int i, j;

  static const int triIdx[][2] = {
    {0,1},{1,2},{2,0}
  };

  static const int quadIdx[][2] = {
    {0,1},{1,2},{2,3},{3,0}
  };

  int verticesPerElement = (containsOnlyTriangles) ? 3 : 4;

  faceList.resize(0);

  for (i=0; i<numElems; i++) {

    for (int k=0; k<verticesPerElement; k++) {
      FieldVector<int, 2> v;
      if (containsOnlyTriangles) {
        v[0] = elemData[verticesPerElement*i+triIdx[k][0]];
        v[1] = elemData[verticesPerElement*i+triIdx[k][1]];
      } else {
        v[0] = elemData[verticesPerElement*i+quadIdx[k][0]];
        v[1] = elemData[verticesPerElement*i+quadIdx[k][1]];
      }
      if (v[0]==v[1])
        continue;

      // Check if new face exists already in the list
      // (then it is no boundary face)
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 2>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0]) ) {
          break;
        }

      }

      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {

        // Insert k-th face of i-th element into face list
        faceList.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)faceList.size(); i++)
    for (j=0; j<2; j++)
      faceList[i][j]--;

}


/** \todo This is quadratic --> very slow */
void Dune::BoundaryExtractor::detectBoundarySegments(int* elemData,
                                                     int numTetras,
                                                     std::vector<FieldVector<int, 3> >& faceList)
{
  int i, j;

  static const int idx[][3] = {
    {3,2,0},{1,2,3},{1,3,0},{2,1,0}
  };

  faceList.resize(0);

  for (i=0; i<numTetras; i++) {

    for (int k=0; k<4; k++) {
      FieldVector<int, 3> v;
      v[0] = elemData[4*i+idx[k][0]];
      v[1] = elemData[4*i+idx[k][1]];
      v[2] = elemData[4*i+idx[k][2]];


      // Check if new face exists already in the list
      // (then it is no boundary face)
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 3>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1] && v[2]==o[2]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0] && v[2]==o[2]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[0]) ||
             (v[0]==o[2] && v[1]==o[0] && v[2]==o[1]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[0]) ) {

          break;
        }

      }

      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {

        // Insert k-th face of i-th tetrahedron into face list
        faceList.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)faceList.size(); i++)
    for (j=0; j<3; j++)
      faceList[i][j]--;

}

/** \todo This is quadratic --> very slow */
void Dune::BoundaryExtractor::detectBoundarySegments(int* elemData,
                                                     int numHexas,
                                                     std::vector<FieldVector<int, 4> >& faceList)
{
  static const int idx[][4] = {
    {0,4,5,1},{1,5,6,2},{2,6,7,3},{3,7,4,0},{4,7,6,5},{1,2,3,0}
  };

  faceList.resize(0);

  for (int i=0; i<numHexas; i++) {

    for (int k=0; k<6; k++) {
      FieldVector<int, 4> v;
      v[0] = elemData[8*i+idx[k][0]] - 1;
      v[1] = elemData[8*i+idx[k][1]] - 1;
      v[2] = elemData[8*i+idx[k][2]] - 1;
      v[3] = elemData[8*i+idx[k][3]] - 1;

      // Don't do anything if the faces is degenerated to a line
      if ((v[0]==v[1] && v[2]==v[3]) ||
          (v[1]==v[2] && v[3]==v[0]) ||
          (v[0]==v[1] && v[1]==v[2]) ||
          (v[1]==v[2] && v[2]==v[3]) ||
          (v[2]==v[3] && v[3]==v[0]) ||
          (v[3]==v[0] && v[0]==v[1]))
        continue;

      // Check whether the faces is degenerated to a triangle

      // Check if new face exists already in the list
      // (then it is no boundary face)
      int j;
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 4>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1] && v[2]==o[2] && v[3]==o[3]) ||
             (v[0]==o[0] && v[1]==o[1] && v[2]==o[3] && v[3]==o[2]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[1] && v[3]==o[3]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[3] && v[3]==o[1]) ||
             (v[0]==o[0] && v[1]==o[3] && v[2]==o[1] && v[3]==o[2]) ||
             (v[0]==o[0] && v[1]==o[3] && v[2]==o[2] && v[3]==o[1]) ||

             (v[0]==o[1] && v[1]==o[0] && v[2]==o[2] && v[3]==o[3]) ||
             (v[0]==o[1] && v[1]==o[0] && v[2]==o[3] && v[3]==o[2]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[0] && v[3]==o[3]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[3] && v[3]==o[0]) ||
             (v[0]==o[1] && v[1]==o[3] && v[2]==o[0] && v[3]==o[2]) ||
             (v[0]==o[1] && v[1]==o[3] && v[2]==o[2] && v[3]==o[0]) ||

             (v[0]==o[2] && v[1]==o[0] && v[2]==o[1] && v[3]==o[3]) ||
             (v[0]==o[2] && v[1]==o[0] && v[2]==o[3] && v[3]==o[1]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[0] && v[3]==o[3]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[3] && v[3]==o[0]) ||
             (v[0]==o[2] && v[1]==o[3] && v[2]==o[0] && v[3]==o[1]) ||
             (v[0]==o[2] && v[1]==o[3] && v[2]==o[1] && v[3]==o[0]) ||

             (v[0]==o[3] && v[1]==o[0] && v[2]==o[1] && v[3]==o[2]) ||
             (v[0]==o[3] && v[1]==o[0] && v[2]==o[2] && v[3]==o[1]) ||
             (v[0]==o[3] && v[1]==o[1] && v[2]==o[0] && v[3]==o[2]) ||
             (v[0]==o[3] && v[1]==o[1] && v[2]==o[2] && v[3]==o[0]) ||
             (v[0]==o[3] && v[1]==o[2] && v[2]==o[0] && v[3]==o[1]) ||
             (v[0]==o[3] && v[1]==o[2] && v[2]==o[1] && v[3]==o[0]) )

          break;
      }



      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {
        // Insert k-th face of i-th hexahedron into face list
        faceList.push_back(v);
      }
    }
  }

  // Rearranging faceList entries that represent triangles
  // They can be recognized by containing an index twice
  for (unsigned int i=0; i<faceList.size(); i++) {

    FieldVector<int,4>& f = faceList[i];

    if (f[0]==f[1]) {
      f[1] = f[2];
      f[2] = f[3];
      f[3] = -1;
    } else if (f[1]==f[2]) {
      f[2] = f[3];
      f[3] = -1;
    } else if (f[2]==f[3]) {
      f[3] = -1;
    } else if (f[0]==f[3]) {
      f[0] = f[1];
      f[1] = f[2];
      f[2] = f[3];
      f[3] = -1;
    } else if (f[0]==f[2] || f[1]==f[3])
      DUNE_THROW(IOError, "Impossible case in detectBoundarySegments");

  }

}


template<int NUM_VERTICES>
int Dune::BoundaryExtractor::detectBoundaryNodes(const std::vector< Dune::FieldVector<int, NUM_VERTICES> >& faceList,
                                                 int noOfNodes,
                                                 std::vector<int>& isBoundaryNode)
{
  isBoundaryNode.resize(noOfNodes);

  int UGNodeIdxCounter = 0;

  for (int i=0; i<noOfNodes; i++)
    isBoundaryNode[i] = -1;

  for (unsigned int i=0; i<faceList.size(); i++) {

    for (int j=0; j<NUM_VERTICES; j++)
      if (faceList[i][j]!=-1 && isBoundaryNode[faceList[i][j]] == -1)
        isBoundaryNode[faceList[i][j]] = 1;

  }

  for (unsigned int i=0; i<isBoundaryNode.size(); i++)
    if (isBoundaryNode[i] != -1)
      isBoundaryNode[i] = UGNodeIdxCounter++;

  return UGNodeIdxCounter;
}


// //////////////////////////////////////////////////////////////////////////////
//   Explicitly instiantiate the member templates that we need.  We need
//   2 and 4, because this is the maximum number of vertices a boundary segment
//   can have in 2d and 3d, respectively.
/** \todo Remove the '3' instantiation */
// //////////////////////////////////////////////////////////////////////////////

template int Dune::BoundaryExtractor::detectBoundaryNodes<2>(const std::vector<FieldVector<int, 2> >& faceList,
                                                             int noOfNodes,
                                                             std::vector<int>& isBoundaryNode);
template int Dune::BoundaryExtractor::detectBoundaryNodes<3>(const std::vector<FieldVector<int, 3> >& faceList,
                                                             int noOfNodes,
                                                             std::vector<int>& isBoundaryNode);
template int Dune::BoundaryExtractor::detectBoundaryNodes<4>(const std::vector<FieldVector<int, 4> >& faceList,
                                                             int noOfNodes,
                                                             std::vector<int>& isBoundaryNode);
