// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRID_HH__
#define __GRID_HH__

namespace Dune {

  /** @defgroup GridCommon Dune Grid Module
          The Dune Grid module defines a general interface to a hierarchical finite element mesh.
          The interface is independent of dimension and element type. Various implementations
          of this interface exits:

          - Structured Grid (SGrid) : A structured mesh in d dimensions consisting of "cubes". The number
          of elements per dimension is variable.

          - Albert Grid (AlbertGrid) : Provides the simplicial meshes of the finite element tool box ALBERT
          written by Kunibert Siebert and Alfred Schmidt.

          - UG Grid (UGGrid) : Provides the meshes of the finite element toolbox UG.

          - Structured Parallel Grid (SPGrid) : Provides a distributed structured mesh.

          This Module contains only the description of compounds that are common to all implementations
          of the grid interface.

          For a detailed description of the interface itself please see the documentation
          of the "Structured Grid Module". Since Dune uses the Engine concept there is no abstract definition
          of the interface. As with STL containers, all implementations must implement the
          same classes with exactly the same members to be used in generic algorithms.

          Class Diagramm: Image inclusion see doxygen doc at page 92.

          @{
   */

  //************************************************************************
  /*! \enum ElementType
      Enum that declares identifiers for different element types. This
      list can be extended in the future. Not all meshes need to implement
      all element types.
   */

  enum ElementType {unknown,line, triangle, quadrilateral, tetrahedron, pyramid, prism, hexahedron,
                    iso_triangle, iso_quadrilateral};

  /** @} */

}


#endif
