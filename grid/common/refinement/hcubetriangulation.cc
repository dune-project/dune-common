// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_REFINEMENT_HCUBETRIANGULATION_CC
#define DUNE_GRID_COMMON_REFINEMENT_HCUBETRIANGULATION_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file contains the @ref Refinement implementation for
         triangulating hypercubes (quadrilateral -> triangle,
         hexahedron -> tetrahedron)

   See @ref HCubeTriangulation.
   @verbatim
   $Id$
   @endverbatim
 */

/*! @defgroup HCubeTriangulation Refinement implementation for triangulating hypercubes
    @ingroup Refinement

   Most stuff here is explained in @ref SimplexRefinement.

   We simply triangulate the hypercube into Kuhn simplices, and than
   use the @ref SimplexRefinement to do the refinement.

   We explicitly use some of the utilities from the @ref SimplexRefinement.

 */

#include "../../../common/fvector.hh"
#include "base.cc"
#include "simplex.cc"

namespace Dune {

  namespace RefinementImp {

    /*! @brief This namespace contains the @ref Refinement
               implementation for triangulating hypercubes
               (quadrilateral -> triangle, hexahedron -> tetrahedron)

       See @ref HCubeTriangulation.
     */
    namespace HCubeTriangulation {

      // ////////////
      //
      //  Utilities
      //

      using Simplex::factorial;
      using Simplex::Factorial;
      using Simplex::binomial;
      using Simplex::getPermutation;
      using Simplex::referenceToKuhn;
      using Simplex::kuhnToReference;

      // ////////////////////////////////////
      //
      //  Refine a hypercube with simplices
      //

      // forward declaration of the iterator base
      template<int dimension, class CoordType, int codimension>
      class RefinementIteratorSpecial;

      template<int dimension_, class CoordType>
      class RefinementImp
      {
      public:
        enum { dimension = dimension_ };

        template<int codimension>
        struct codim;
        typedef typename codim<dimension>::SubEntityIterator VertexIterator;
        typedef FieldVector<CoordType, dimension> CoordVector;
        typedef typename codim<0>::SubEntityIterator ElementIterator;
        typedef FieldVector<int, dimension+1> IndexVector;

        static int nVertices(int level);
        static VertexIterator vBegin(int level);
        static VertexIterator vEnd(int level);

        static int nElements(int level);
        static ElementIterator eBegin(int level);
        static ElementIterator eEnd(int level);
      private:
        friend class RefinementIteratorSpecial<dimension, CoordType, 0>;
        friend class RefinementIteratorSpecial<dimension, CoordType, dimension>;

        typedef Simplex::RefinementImp<dimension, CoordType> BackendRefinement;
      };

      template<int dimension, class CoordType>
      template<int codimension>
      struct RefinementImp<dimension, CoordType>::codim
      {
        class SubEntityIterator;
      };

      template<int dimension, class CoordType>
      int
      RefinementImp<dimension, CoordType>::
      nVertices(int level)
      {
        return BackendRefinement::nVertices(level) * Factorial<dimension>::value;
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::VertexIterator
      RefinementImp<dimension, CoordType>::
      vBegin(int level)
      {
        return VertexIterator(level);
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::VertexIterator
      RefinementImp<dimension, CoordType>::
      vEnd(int level)
      {
        return VertexIterator(level, true);
      }

      template<int dimension, class CoordType>
      int
      RefinementImp<dimension, CoordType>::
      nElements(int level)
      {
        return BackendRefinement::nElements(level) * Factorial<dimension>::value;
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::ElementIterator
      RefinementImp<dimension, CoordType>::
      eBegin(int level)
      {
        return ElementIterator(level);
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::ElementIterator
      RefinementImp<dimension, CoordType>::
      eEnd(int level)
      {
        return ElementIterator(level, true);
      }

      // //////////////
      //
      // The iterator
      //

      // vertices

      template<int dimension, class CoordType>
      class RefinementIteratorSpecial<dimension, CoordType, dimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef typename Refinement::CoordVector CoordVector;

        RefinementIteratorSpecial(int level, bool end = false);

        void increment();

        CoordVector coords() const;
        int index() const;
      protected:
        typedef typename Refinement::BackendRefinement BackendRefinement;
        typedef typename BackendRefinement::template codim<dimension>::SubEntityIterator BackendIterator;
        enum { nKuhnSimplices = Factorial<dimension>::value };

        int level;

        int kuhnIndex;
        BackendIterator backend;
        const BackendIterator backendEnd;
      };

      template<int dimension, class CoordType>
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      RefinementIteratorSpecial(int level_, bool end)
        : level(level_), kuhnIndex(0),
          backend(BackendRefinement::vBegin(level)),
          backendEnd(BackendRefinement::vEnd(level))
      {
        if(end)
          kuhnIndex = nKuhnSimplices;
      }

      template<int dimension, class CoordType>
      void
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      increment()
      {
        ++backend;
        if(backend == backendEnd) {
          backend = BackendRefinement::vBegin(level);
          ++kuhnIndex;
        }
      }

      template<int dimension, class CoordType>
      typename RefinementIteratorSpecial<dimension, CoordType, dimension>::CoordVector
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      coords() const
      {
        return referenceToKuhn(backend.coords(), getPermutation<dimension>(kuhnIndex));
      }

      template<int dimension, class CoordType>
      int
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      index() const
      {
        return kuhnIndex*BackendRefinement::nVertices(level) + backend.index();
      }

      // elements

      template<int dimension, class CoordType>
      class RefinementIteratorSpecial<dimension, CoordType, 0>
      {
      public:
        typedef RefinementImp<dimension, CoordType> RefinementImp;
        typedef typename RefinementImp::IndexVector IndexVector;

        RefinementIteratorSpecial(int level, bool end = false);

        void increment();

        IndexVector vertexIndices() const;
        int index() const;
      protected:
        typedef typename RefinementImp::BackendRefinement BackendRefinement;
        typedef typename BackendRefinement::template codim<0>::SubEntityIterator BackendIterator;
        enum { nKuhnSimplices = Factorial<dimension>::value };

        int level;

        int kuhnIndex;
        BackendIterator backend;
        const BackendIterator backendEnd;
      };

      template<int dimension, class CoordType>
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      RefinementIteratorSpecial(int level_, bool end)
        : level(level_), kuhnIndex(0),
          backend(BackendRefinement::eBegin(level)),
          backendEnd(BackendRefinement::eEnd(level))
      {
        if(end)
          kuhnIndex = nKuhnSimplices;
      }

      template<int dimension, class CoordType>
      void
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      increment()
      {
        ++backend;
        if(backend == backendEnd) {
          backend = BackendRefinement::eBegin(level);
          ++kuhnIndex;
        }
      }

      template<int dimension, class CoordType>
      typename RefinementIteratorSpecial<dimension, CoordType, 0>::IndexVector
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      vertexIndices() const
      {
        IndexVector indices = backend.vertexIndices();

        int base = kuhnIndex * BackendRefinement::nVertices(level);
        indices += base;

        return indices;
      }

      template<int dimension, class CoordType>
      int
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      index() const
      {
        return kuhnIndex*BackendRefinement::nElements(level) + backend.index();
      }

      // common

      template<int dimension, class CoordType>
      template<int codimension>
      class RefinementImp<dimension, CoordType>::codim<codimension>::SubEntityIterator
        : public ForwardIteratorFacade<typename RefinementImp<dimension, CoordType>::template codim<codimension>::SubEntityIterator, int>,
          public RefinementIteratorSpecial<dimension, CoordType, codimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef SubEntityIterator This;

        SubEntityIterator(int level, bool end = false);

        bool equals(const This &other) const;
      protected:
        using RefinementIteratorSpecial<dimension, CoordType, codimension>::kuhnIndex;
        using RefinementIteratorSpecial<dimension, CoordType, codimension>::backend;
      };

      template<int dimension, class CoordType>
      template<int codimension>
      RefinementImp<dimension, CoordType>::codim<codimension>::SubEntityIterator::
      SubEntityIterator(int level, bool end)
        : RefinementIteratorSpecial<dimension, CoordType, codimension>(level, end)
      {}

      template<int dimension, class CoordType>
      template<int codimension>
      bool
      RefinementImp<dimension, CoordType>::codim<codimension>::SubEntityIterator::
      equals(const This &other) const
      { return kuhnIndex == other.kuhnIndex && backend == other.backend; }

    } // namespace HCubeTriangulation

    // ///////////////////////
    //
    // The refinement traits
    //

    template<class CoordType>
    struct Traits<quadrilateral, CoordType, triangle>
    {
      typedef HCubeTriangulation::RefinementImp<2, CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<iso_quadrilateral, CoordType, triangle>
    {
      typedef HCubeTriangulation::RefinementImp<2, CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<quadrilateral, CoordType, iso_triangle>
    {
      typedef HCubeTriangulation::RefinementImp<2, CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<iso_quadrilateral, CoordType, iso_triangle>
    {
      typedef HCubeTriangulation::RefinementImp<2, CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<hexahedron, CoordType, tetrahedron>
    {
      typedef HCubeTriangulation::RefinementImp<3, CoordType> Imp;
    };

  } // namespace RefinementImp

} // namespace Dune

#endif //DUNE_GRID_COMMON_REFINEMENT_HCUBETRIANGULATION_CC
