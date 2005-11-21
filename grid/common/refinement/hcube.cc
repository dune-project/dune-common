// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_REFINEMENT_HCUBE_CC
#define DUNE_GRID_COMMON_REFINEMENT_HCUBE_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file contains the @ref Refinement implementation for
         hypercubes (quadrilaterals, hexahedrons, etc.).

   See @ref HCubeRefinement.
   @verbatim
   $Id$
   @endverbatim
 */

/*! @defgroup HCubeRefinement Refinement implementation for hypercubes
    @ingroup Refinement

   This @ref Refinement implementation uses an SGrid as it's backend.
   The SGrid is wrapped by @ref RefinementGrid to make it singleton.
   RefinementImp than adapts the SGrid interface to the @ref Refinement
   interface.

   @section Iterators The Iterators
   <!--=========================-->

   For the iterators we have to hack around a bit.  The problem is as
   follows:
   @code
   template<int A>
   class outer
   {
    template<int B>
    class inner;
   };
   @endcode
   C++ does not allow specialisation of the inner class when the outer
   class is not specialized.

   So I had to create a baseclass for the iterators which is not inside
   another class.  This base class can then be specialized, and the
   real Iterator class inherits from it.  I gave it the somewhat clumsy
   name RefinementSubEntityIteratorSpecial.
 */

#include "../../../common/fvector.hh"
#include "../../sgrid.hh"
#include "../grid.hh"
#include "../../../common/iteratorfacades.hh"
#include "base.cc" // for RefinementTraits

namespace Dune {

  namespace RefinementImp {

    /*! @brief This namespace contains the @ref Refinement implementation
       for hypercubes (quadrilaterals, hexahedrons, etc.).

       See @ref HCubeRefinement.
     */
    namespace HCube {

      //
      // refinement implementation for hypercubes
      //

      template<int dimension>
      class RefinementGrid;

      /*! @brief @ref Refinement implementation for hypercubes

          @param dimension_ Dimension of the refined hypercube
          @param CoordType  Coordinate type of the refined hypercube

          We use @ref RefinementGrid as backend to do all the work.

          The interface is the same as for @ref Dune::Refinement (apart
          from the template parameters).
       */
      template<int dimension_, class CoordType>
      class RefinementImp
      {
      public:
        enum { dimension = dimension_ /*!< Know your own dimension @hideinitializer */ };
        //- Know yourself
        typedef RefinementImp<dimension, CoordType> Refinement;

        template<int codimension>
        struct Codim;
        typedef typename Codim<dimension>::SubEntityIterator VertexIterator;
        typedef FieldVector<CoordType, dimension> CoordVector;
        typedef typename Codim<0>::SubEntityIterator ElementIterator;
        typedef FieldVector<int, (1<<dimension)> IndexVector;

        static int nVertices(int level);
        static VertexIterator vBegin(int level);
        static VertexIterator vEnd(int level);

        static int nElements(int level);
        static ElementIterator eBegin(int level);
        static ElementIterator eEnd(int level);
      private:
        //- Know the backend grid
        typedef RefinementGrid<dimension> Grid;
      };

      template<int dimension, class CoordType>
      template<int codimension>
      struct RefinementImp<dimension, CoordType>::Codim
      {
        class SubEntityIterator;
        typedef typename RefinementGrid<dimension>::BaseType::template Codim<codimension>::Geometry Geometry;
      };

      template<int dimension, class CoordType>
      int
      RefinementImp<dimension, CoordType>::
      nVertices(int level)
      {
        Grid::instance().refineTo(level);
        return Grid::instance().size(level, dimension);
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::VertexIterator
      RefinementImp<dimension, CoordType>::
      vBegin(int level)
      {
        Grid::instance().refineTo(level);
        return VertexIterator(Grid::instance().template lbegin<dimension>(level));
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::VertexIterator
      RefinementImp<dimension, CoordType>::
      vEnd(int level)
      {
        Grid::instance().refineTo(level);
        return VertexIterator(Grid::instance().template lend<dimension>(level));
      }

      template<int dimension, class CoordType>
      int
      RefinementImp<dimension, CoordType>::
      nElements(int level)
      {
        Grid::instance().refineTo(level);
        return Grid::instance().size(level, 0);
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::ElementIterator
      RefinementImp<dimension, CoordType>::
      eBegin(int level)
      {
        Grid::instance().refineTo(level);
        return ElementIterator(Grid::instance().template lbegin<0>(level));
      }

      template<int dimension, class CoordType>
      typename RefinementImp<dimension, CoordType>::ElementIterator
      RefinementImp<dimension, CoordType>::
      eEnd(int level)
      {
        Grid::instance().refineTo(level);
        return ElementIterator(Grid::instance().template lend<0>(level));
      }

      //
      // The backend Grid
      //

      /*! @brief Backend grid for hypercube refinement

          @param dimension Dimension of the refined hypercube

          This grid is used as backend by @ref RefinementImp.  It simply
          wraps an SGrid to make it a singleton.  We have to use SGrids
          default CoordType here instead of the one from the refined
          hypercube, because I know of no way to set the CoordType used
          by SGrid.
       */
      template<int dimension>
      class RefinementGrid : public SGrid<dimension, dimension>
      {
      public:
        //! Know yourself
        typedef RefinementGrid<dimension> This;
        //! Know your base class
        typedef SGrid<dimension, dimension> BaseType;

        //! Make sure the grid as at least the given refinement level
        void refineTo(int level);
        //! Return the singleton instance
        static This &instance();
      private:
        RefinementGrid();

        static This *instance_;
      };

      /*!
                This simply wraps the globalRefine() method of SGrid.
       */
      template<int dimension>
      void RefinementGrid<dimension>::refineTo(int level /*! The refinement level to enforce */)
      {
        if(SGrid<dimension, dimension>::maxLevel() < level)
          globalRefine(level - SGrid<dimension, dimension>::maxLevel());
      }


      /*!
                Return the singleton instance of the RefinementGrid.  Create it if neccessary.
       */
      template<int dimension>
      RefinementGrid<dimension> &
      RefinementGrid<dimension>::instance()
      {
        if(instance_ == 0)
          instance_ = new This;
        return *instance_;
      }

      template<int dimension>
      RefinementGrid<dimension>::RefinementGrid()
      {}

      template<int dimension>
      RefinementGrid<dimension> *
      RefinementGrid<dimension>::instance_ = 0;

      //
      // The iterator
      //

#ifdef DOXYGEN
      /*! @brief SubEntityIterator base class for hypercube refinement

          @param dimension   Dimension of the refined element
          @param CoordType   Coordinate type of the refined element
          @param codimension Codimension of the iterator

          This is the base class for SubEntityIterators.  We have to use
          this construct because RefinementImp<...>::codim<...> cannot
          be specialized without first specializing RefinementImp.
       */
      template<int dimension, class CoordType, int codimension>
      class RefinementSubEntityIteratorSpecial {};
#else //!DOXYGEN
      template<int dimension, class CoordType, int codimension>
      class RefinementSubEntityIteratorSpecial;
#endif //DOXYGEN

      // for vertices

      template<int dimension, class CoordType>
      class RefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef typename Refinement::template Codim<dimension>::SubEntityIterator Common;
        typedef typename Refinement::CoordVector CoordVector;

        CoordVector coords() const;
      };

      template<int dimension, class CoordType>
      typename RefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>::CoordVector
      RefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>::
      coords() const
      {
        // Assume a vertex has exactly one corner
        // Assume the reference element an n-cube has all coordinates ranging from 0 to 1
        return static_cast<const Common*>(this)->backend->geometry()[0];
      }

      // for elements

      template<int dimension, class CoordType>
      class RefinementSubEntityIteratorSpecial<dimension, CoordType, 0>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef typename Refinement::template Codim<0>::SubEntityIterator Common;
        typedef typename Refinement::IndexVector IndexVector;

        IndexVector vertexIndices() const;
      };

      template<int dimension, class CoordType>
      typename RefinementSubEntityIteratorSpecial<dimension, CoordType, 0>::IndexVector
      RefinementSubEntityIteratorSpecial<dimension, CoordType, 0>::
      vertexIndices() const
      {
        enum { nIndices = (1 << dimension) };

        assert(nIndices == static_cast<const Common*>(this)->backend->template count<dimension>());
        IndexVector vec;
        for(int i = 0; i < nIndices; ++i)
          vec[i] = RefinementGrid<dimension>::instance().levelIndexSet(static_cast<const Common*>(this)->backend->level()).template subIndex<dimension>(*(static_cast<const Common*>(this)->backend),nIndices - i - 1);
        return vec;
      }



      // common

      template<int dimension, class CoordType>
      template<int codimension>
      class RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator
        : public ForwardIteratorFacade<typename RefinementImp<dimension, CoordType>::template Codim<codimension>::SubEntityIterator, int>,
          public RefinementSubEntityIteratorSpecial<dimension, CoordType, codimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef typename Refinement::template Codim<codimension>::SubEntityIterator This;
        typedef typename RefinementGrid<dimension>::template Codim<codimension>::LevelIterator
        BackendIterator;

        SubEntityIterator(const BackendIterator &backend);

        bool equals(const This &other) const;
        void increment();

        int index() const;
        const Geometry &geometry() const;
      private:
        friend class RefinementSubEntityIteratorSpecial<dimension, CoordType, codimension>;
        BackendIterator backend;
      };

      template<int dimension, class CoordType>
      template<int codimension>
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      SubEntityIterator(const BackendIterator &backend_)
        : backend(backend_)
      {}

      template<int dimension, class CoordType>
      template<int codimension>
      bool
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      equals(const This &other) const
      { return backend == other.backend; }

      template<int dimension, class CoordType>
      template<int codimension>
      void
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      increment()
      {
        ++backend;
      }

      template<int dimension, class CoordType>
      template<int codimension>
      int
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      index() const
      { return RefinementGrid<dimension>::instance().levelIndexSet(backend->level()).index(*backend); }
      //      { return backend->index(); }



      template<int dimension, class CoordType>
      template<int codimension>
      const typename RefinementImp<dimension, CoordType>::template Codim<codimension>::Geometry &
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      geometry() const
      { return backend->geometry(); }

    } // namespace HCube

    // ///////////////////////
    //
    // The refinement traits
    //

    template<class CoordType>
    struct Traits<quadrilateral, CoordType, quadrilateral>
    {
      typedef HCube::RefinementImp<2, CoordType> Imp;
    };

    template<class CoordType>
    struct Traits<hexahedron, CoordType, hexahedron>
    {
      typedef HCube::RefinementImp<3, CoordType> Imp;
    };

  } // namespace RefinementImp

} // namespace Dune

#endif //DUNE_GRID_COMMON_REFINEMENT_HCUBE_CC
