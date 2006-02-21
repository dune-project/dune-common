// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_REFINEMENT_HH
#define DUNE_GRID_COMMON_REFINEMENT_HH

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file simply includes all @ref Refinement implementations
         so you don't have to do them seperately.

   @verbatim
   $Id$
   @endverbatim
 */

/*! @defgroup Refinement Refinement
    @ingroup Grid
    <!--WWWWWWWWWWWWW-->

   @section General
   <!--=========-->

   The Refinement system allows to temporarily refine a grid or single
   entities without changing the grid itself.  You may want to do this
   because you want to write your data to a file and have to do
   subsampling, but want to continue the calculation with the
   unmodified grid afterwards.

   @subsection Can_do What Refinement can do for you
   <!---------------------------------------------->

   For a given geometry type and refinement level, Refinement will
   - assign consecutive integer indices starting at 0 to each
    subvertex,
   - assign consecutive integer indices starting at 0 to each
    subelement,
   - calculate the coordinates of the subvertices for you,
   - calculate subvertex-indices of the corners of the subelements for
    you.

   The geometry type of the refined entity and of the subelements may
   be different, for example you can refine a quadrilateral but get
   subelements which are triangles.

   Currently the following geometry types are supportet:
   - hypercubes (quadrilaterals, hexahedrons),
   - simplices (triangles, tetrahedrons),
   - triangulating hypercubes into simplices (quadrilaterals ->
    triangles, hexahedrons -> tetrahedrons).

   @subsection Cannot_do What Refinement can't do for you
   <!--------------------------------------------------->

   - Refinement does not actually subsample your data, it only tells
    you @em where to subsample your data.
   - The geometry types need to be known at compile time.  See @link
    VirtualRefinement VirtualRefinement@endlink if you need to
    calculate the right geometry type at run time.
   - No Refinement implementations for anything besides hypercubes and
    simplices have been written yet.

   @section User_interface The user interface
   <!--===================================-->

   @code
   template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension>
   class Refinement
   {
   public:
    enum { dimension };

    template<int codimension>
    struct codim {
      class SubEntityIterator;
    };
    typedef VertexIterator;  // These are aliases for codim<codim>::SubEntityIterator
    typedef ElementIterator;

    typedef IndexVector; // These are FieldVectors
    typedef CoordVector;

    static int nVertices(int level);
    static VertexIterator vBegin(int level);
    static VertexIterator vEnd(int level);

    static int nElements(int level);
    static ElementIterator eBegin(int level);
    static ElementIterator eEnd(int level);
   }
   @endcode

   The Iterators can do all the usual things that Iterators can do,
   except dereferencing.  In addition, to do something useful, they
   support some additional methods:

   @code
   template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension>
   class VertexIterator
   {
   public:
    typedef Refinement;

    int index() const;
    Refinement::CoordVector coords() const;
   }

   template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension>
   class ElementIterator
   {
   public:
    typedef Refinement;

    int index() const;
    Refinement::IndexVector vertexIndices() const;
   }
   @endcode

   @subsection How_to_use_it How to use it
   <!------------------------------------>

   Either use VirtualRefinement, or if you don't want to do that, read
   on.

   @code
   // Include the neccessary files
   #include <dune/grid/common/refinement.hh>

   // If you know that you are only ever going to need one refinement
   // backend, you can include the corresponding file directly:
   //#include <dune/grid/common/refinement/hcube.cc>

   // Get yourself the Refinement you need:
   typedef Refinement<GeometryType::cube, SGrid<2, 2>::ctype, GeometryType::cube, 2> MyRefinement;

   int main()
   {
    const int refinementlevel = 2;
    cout << "Using refinementlevel = " << refinementlevel << endl << endl;

    // get Number of Vertices
    cout << "Number of Vertices: "
         << MyRefinement::nVertices(refinementlevel)
         << endl;

    // Iterate over Vertices
    cout << "Index\tx\ty" << endl;
    MyRefinement::VertexIterator vEnd = MyRefinement::vEnd(refinementlevel);
    for(MyRefinement::VertexIterator i = MyRefinement::vBegin(refinementlevel); i != vEnd; ++i)
      cout << i.index() << "\t" << i.coords()[0] << "\t" << i.coords()[1] << endl;
    cout << endl;

    // Iterate over Vertices
    cout << "Index\tEcke0\tEcke1\tEcke2\tEcke3" << endl;
    MyRefinement::ElementIterator eEnd = MyRefinement::eEnd(refinementlevel);
    for(MyRefinement::ElementIterator i = MyRefinement::eBegin(refinementlevel); i != eEnd; ++i)
      cout << i.index() << "\t"
           << i.indexVertices()[0] << "\t" << i.indexVertices()[1] << "\t"
           << i.indexVertices()[2] << "\t" << i.indexVertices()[3] << endl;
    cout << endl;
   }
   @endcode

   @subsection Guarantees
   <!------------------->

   The Refinement system gives this guarantee (besides conforming to
   the above interface:
   - The indices of the subvertices and subelement start at 0 and are
    consecutive.

   @section Implementing Implementing a new Refinement type
   <!--=================================================-->

   If you want to write a Refinement implementation for a particular
   geometry type, e.g. SquaringTheCircle (or a particular set of
   geometry types) here is how:

   - create a file refinement/squaringthecircle.cc and #include
    "base.cc".  Your file will be included by others, so don't forget
    to protect against double inclusion.
   - implement a class (or template class) RefinementImp conforming
    exactly to the user interface above.
   - put it (and it's helper stuff as apropriate) into it's own
    namespace Dune::RefinementImp::SquaringTheCircle.
   - define the mapping of geometryType, CoordType and coerceTo to your
    implementation by specialising template struct
    RefinementImp::Traits.  It should look like this:
    @code
   namespace Dune::RefinementImp {
    // the "dim" template parameter is ignored, since the dimension can be infered
    template<class CoordType>
    struct Traits<GeometryType::sphere, CoordType, GeometryType::cube, 2> {
      typedef SquaringTheCircle::RefinementImp<CoordType> Imp;
    };

    // we're only implementing this for dim=2
    template<class CoordType>
    struct Traits<GeometryType::sphere, CoordType, GeometryType::cube, 2> {
      typedef SquaringTheCircle::RefinementImp<CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<GeometryType::circle, CoordType, GeometryType::cube, 2> {
      typedef SquaringTheCircle::RefinementImp<CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<GeometryType::sphere, CoordType, GeometryType::quadrilateral, 2> {
      typedef SquaringTheCircle::RefinementImp<CoordType> Imp;
    };
   }
    @endcode
    If you implement a template class, you have to specialise struct
    RefinementImp::Traits for every possible combination of
    geometryType and coerceTo that your implementation supports.
   - #include "refinement/squaringthecircle.cc" from refinement.hh.

   This is enough to integrate your implementation into the Refinement
   system.  You probably want to include it into @link
   VirtualRefinement VirtualRefinement@endlink also.

   @subsection Namespaces
   <!------------------->

   The (non-virtual) Refinement namespaces is organized in the
   following way:
   - Only template class Refinement lives directly in namespace Dune.
   - Use namespace Dune::RefinementImp for all the Implementation.
   - Use template struct Dune::RefinementImp::Traits instead of
    template struct Dune::RefinementTraits.
   - Use namespace Dune::RefinementImp::HCube, namespace
    Dune::RefinementImp::Simplex, ... for each implementation.

   The complete @link VirtualRefinement VirtualRefinement@endlink stuff
   is directly in namespace Dune.

   @subsection Layers Conceptual layers
   <!--------------------------------->

   - <strong>Layer 0</strong> declares struct
    RefinementImp::Traits<geometryType, CoordType, coerceTo, dim>.
    It's member typedef Imp tells which Refinement implementation to
    use for a given geometryType (and CoordType).  It is located in
    refinementbase.cc.
   - <strong>Layer 1</strong> defines
    RefinementImp::XXX::RefinementImp.  It implements the Refinements
    for each geometryType, coerceTo (and CoordType).  Also in this
    layer are the definitions of struct RefinementImp::Traits.  This
    layer is located in refinementXXX.cc.
   - <strong>Layer 2</strong> puts it all together.  It defines class
    Refinement<geometryType, CoordType, coerceTo, dim> by deriving
    from the corresponding RefinementImp.  It is located in
    refinementbase.cc.
   - There is a dummy <strong>layer 2.5</strong> which simply includes
    all the refinementXXX.cc files.  It is located in refinement.cc.

   @link VirtualRefinement VirtualRefinement@endlink adds two more
   layers to the ones defined here.

 */

// The interface (template<...> class Refinement) is not included here
// since it derives from parts which I consider implementation.  Look
// into refinement/base.cc if the documentation is above is not enough.
#include "refinement/base.cc"

#include "refinement/hcube.cc"
#include "refinement/simplex.cc"
#include "refinement/hcubetriangulation.cc"

#endif //DUNE_GRID_COMMON_REFINEMENT_HH
