// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_EXTRACTOR_HH
#define DUNE_BOUNDARY_EXTRACTOR_HH

#include <vector>
#include <dune/common/fvector.hh>


namespace Dune {

  class BoundaryExtractor {

  public:

    static void detectBoundarySegments(int* elemData,
                                       int numElems,
                                       std::vector<FieldVector<int, 2> >& faceList,
                                       bool containsOnlyTriangles);

    /** \todo Remove this once UGGrid controls this class */
    static void detectBoundarySegments(int* elemData,
                                       int numTetras,
                                       std::vector<FieldVector<int, 3> >& faceList);

    static void detectBoundarySegments(int* elemData,
                                       int numHexas,
                                       std::vector<FieldVector<int, 4> >& faceList);

    template <int NumVertices>
    static int detectBoundaryNodes(const std::vector<FieldVector<int, NumVertices> >& faceList,
                                   int noOfNodes,
                                   std::vector<int>& isBoundaryNode);

  };

}

#endif
