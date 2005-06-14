// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_REFERENCE_TOPOLOGY_HH
#define DUNE_REFERENCE_TOPOLOGY_HH

#include <dune/grid/common/grid.hh>
#include <dune/common/exceptions.hh>

/** \file
 * \brief The reference topologies
 */

namespace Dune {

  /** \brief The reference topologies of the different element types
   * \ingroup GridCommon
   */
  class ReferenceTopologySet {

    static const int lineVerticesPerFace[2];

    static const int triangleVerticesPerFace[3][2];

    static const int quadrilateralVerticesPerFace[4][2];

    static const int tetrahedronVerticesPerFace[4][3];

    static const int pyramidVerticesPerFace[5][4];

    static const int prismVerticesPerFace[5][4];

    static const int hexahedronVerticesPerFace[6][4];


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


}  // end namespace Dune

#endif
