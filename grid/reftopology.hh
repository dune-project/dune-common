// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_REFERENCE_TOPOLOGY_HH
#define DUNE_REFERENCE_TOPOLOGY_HH

/** \file
 * \brief The reference topologies
 */

namespace Dune {

  /** \brief The reference topologies of the different element types
   * \ingroup GridCommon
   */
  class ReferenceTopologySet {

    static const int ReferenceTopologySet::lineVerticesPerFace[2];

    static const int ReferenceTopologySet::triangleVerticesPerFace[3][2];

    static const int ReferenceTopologySet::quadrilateralVerticesPerFace[4][2];

    static const int ReferenceTopologySet::tetrahedronVerticesPerFace[4][3];

    static const int ReferenceTopologySet::pyramidVerticesPerFace[5][4];

    static const int ReferenceTopologySet::prismVerticesPerFace[5][4];

    static const int ReferenceTopologySet::hexahedronVerticesPerFace[6][4];


  public:

    /** \brief Returns subface information
     *
     * This method returns information about subentities of subentities of
     * an element.  For example,  say you have a three-dimensional element
     * <tt> e </tt> and you're interested in the vertices of its second
     * face.  Then note that vertices have codimension three and faces have
     * codimension one and write
     * \verbatim
       int n;
       const int* faceIdx;
       getSubEntities<1,3>(e, 2, faceIdx, n)
       \endverbatim
     * Afterwards, <tt>faceIdx</tt> will contain a pointer to the local
     * vertices and <tt>n</tt> will contain the number of those vertices.
     */
    template <int inputCodim, int outputCodim>
    static void getSubEntities(const GeometryType& eType,
                               int entity,
                               const int*& result,
                               int& n);

  };

  template <int inputCodim, int outputCodim>
  void ReferenceTopologySet::getSubEntities(const GeometryType& eType,
                                            int entity,
                                            const int*& result,
                                            int& n)
  {
    DUNE_THROW(NotImplemented, "no subEntity information for these codims");
  }


  template <>
  void ReferenceTopologySet::getSubEntities<1,1>(const GeometryType& eType,
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
  void ReferenceTopologySet::getSubEntities<1,2>(const GeometryType& eType,
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
  void ReferenceTopologySet::getSubEntities<1,3>(const GeometryType& eType,
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

  const int ReferenceTopologySet::lineVerticesPerFace[2] = {0,1};

  const int ReferenceTopologySet::triangleVerticesPerFace[3][2] = { {1,2}, {2,0}, {0,1} };

  const int ReferenceTopologySet::quadrilateralVerticesPerFace[4][2] = { {2,0}, {1,3} , {0,1} , {3,2}};

  const int ReferenceTopologySet::tetrahedronVerticesPerFace[4][3] = { {1, 2, 3},
                                                                       {0, 3, 2},
                                                                       {0, 1, 3},
                                                                       {0, 2, 1} };

  const int ReferenceTopologySet::pyramidVerticesPerFace[5][4] = { {0, 3, 2, 1},
                                                                   {0, 1, 4, -1},
                                                                   {1, 2, 4, -1},
                                                                   {2, 3, 4, -1},
                                                                   {0, 4, 3, -1} };

  const int ReferenceTopologySet::prismVerticesPerFace[5][4] = { {0, 2, 1, -1},
                                                                 {0, 1, 4, 3},
                                                                 {1, 2, 5, 4},
                                                                 {0, 3, 5, 2},
                                                                 {3, 4, 5, -1} };


  const int ReferenceTopologySet::hexahedronVerticesPerFace[6][4] = { {0, 4, 6, 2},
                                                                      {1, 3, 7, 5},
                                                                      {0, 1, 5, 4},
                                                                      {2, 6, 7, 3},
                                                                      {0, 2, 3, 1},
                                                                      {5, 7, 6, 4} };

}

#endif
