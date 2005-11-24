// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_EXTRACTOR_HH
#define DUNE_BOUNDARY_EXTRACTOR_HH

/** \file
    \brief Contains a helper class for the creation of UGGrid objects
    \author Oliver Sander
 */

#include <vector>
#include <dune/common/fvector.hh>


namespace Dune {

  class BoundaryExtractor {

  public:

    static void detectBoundarySegments(std::vector<unsigned char> elementTypes,
                                       std::vector<unsigned int> elementVertices,
                                       std::vector<FieldVector<int, 2> >& boundarySegments);

    static void detectBoundarySegments(std::vector<unsigned char> elementTypes,
                                       std::vector<unsigned int> elementVertices,
                                       std::vector<FieldVector<int, 4> >& faceList);

    template <int NumVertices>
    static int detectBoundaryNodes(const std::vector<FieldVector<int, NumVertices> >& faceList,
                                   int noOfNodes,
                                   std::vector<int>& isBoundaryNode);

  };

}

#endif
