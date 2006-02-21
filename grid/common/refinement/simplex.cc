// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_REFINEMENT_SIMPLEX_CC
#define DUNE_GRID_COMMON_REFINEMENT_SIMPLEX_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file contains the @ref Refinement implementation for
         simplices (triangles, tetrahedrons...)

   See @ref SimplexRefinement.
   @verbatim
   $Id$
   @endverbatim
 */

/*! @defgroup SimplexRefinement Refinement implementation for simplices
    @ingroup Refinement

   This is mainly based on Jürgen Beys
   <http://www.igpm.rwth-aachen.de/bey/> dissertation.  The relevant
   part is available from
   <http://www.igpm.rwth-aachen.de/bey/ftp/simplex.ps.gz>.

   @section Terminology
   <!--=============-->

   <dl>
   <dt>Kuhn simplex</dd>
   <dd>To triangulate hypercubes we use the Kuhn triangulation.  The
      members of this triangulation we call <em>Kuhn simplices</em>.
      The Kuhn simplices are indexed by their corresponding
      permutation.</dd>
   <dt>Kuhn0 simplex</dt>
   <dd>The Kuhn simplex corresponding to the permutation number 0.</dd>
   <dt>size of a Kuhn simplex</dt>
   <dd>The size of a kuhn simplex is equal to the size of the hypercube
      that it triangulates.</dd>
   <dt>width of a Kuhn simplex</dt>
   <dd> See <em>size of a Kuhn simplex</em>.</dd>
   </dl>

   @section KuhnSimplexIndexing Describing Kuhn simplices by their permutation
   <!--====================================================================-->

   A Kuhn simplex of dimension n can be described by its size s and a
   permutation of the vector \f$\vec{p}=(0,\ldots,n-1)\f$.  To get the
   coordinates of the corners \f$\vec{x}_0,\ldots,\vec{x}_n\f$ of the
   simplex, you do as follows:
   - Start at the origin \f$\vec{x}_0\f$.
   - For each dimension d from 0 to n-1:
    - \f$\vec{x}_{d+1}:=\vec{x}_d+s\cdot\vec{e}_{p_d}\f$
   (\f$\vec{e}_i\f$ is the unit vector in direction i.)

   @section Kuhn0VertexCounting Number of vertices in a Kuhn0 simplex
   <!--===========================================================-->

   Let N(n, x) be the number of gridpoints within an n-dimensional
   Kuhn0 simplex of x gridunits width.

   The number of points in a 0-dimensional simplex is 1, independent of
   its width.

   N(0, x) = 1

   The recursion formula is
   <!--
               x
              --
   N(n+1, x) = >  N(n, i)
              --
              i=0
   -->
   \f[N(n+1,x)=\sum^x_{i=0}N(n,i)\f]

   We slice the n+1 dimensional simplex ortogonal to one of the
   dimensions and sum the number of points in the n dimensional
   subsimplices.

   This formula is satisfied by the binomial koefficient
   <!--
            ( n+x )
   N(n, x) = (     )
            (  n  )
   -->
   \f[N(n,x)=\left({n+x}\atop x\right)\f]

   See Bronstein, Semendjajew, Musiol, Mühlig "Taschenbuch der
   Mathematik" (1999), Formula (1.35c)

   Observations:
   - N(n, 0) = 1
   - N(n, x) = N(x, n)

   @section Kuhn0VertexIndexing Index of a vertex within a Kuhn0 simplex
   <!--==============================================================-->

   @image html simplexvertexindex.png "The image shows the Kuhn0 tetrahedron of width 2 (wireframe).  It is partitioned into a tetrahedron (green), a triangle (red), a line (blue), and a vertex (black), each of width 1 and each a Kuhn0 simplex."

   Let us calculate the index of vertex 9, which has the coordinates
   \f$(x_0,x_1,x_2)=(2,2,2)\f$.
   - First we count the number of vertices in the green tetrahedron of
    width \f$x_0-1=1\f$ (4).  Then we take the green tetrahedron away.
    Whats left is a triangle, which extends into the (1,2)-plane.
   - Now we count the number of vertices in the red triangle of width
    \f$x_1-1=1\f$ (3).  Again we take the counted points away and are
    left with a line which extends into direction 2.
   - We take the blue line of width \f$x_2-1=1\f$, count the number of
    vertices (2), and throw away the counted stuff.  The only thing
    remaining is a point, so we're done.
   - We add the counted stuff together and get indeed 9.

   On to a more complicated example: vertex 6 with coordinates
   \f$(x_0,x_1,x_2)=(2,1,1)\f$.
   - First count the vertices in the green tetrahedron again (width
    \f$x_0-1=1\f$). The result is 4.
   - Count the vertices in the triangle of width \f$x_1-1=0\f$ (vertex
    4), which is just a point.  The result is 1.
   - Count the vertices in the line of width \f$x_2-1=0\f$ (vertex 5),
    which is also just a point.  The result is 1.
   - Add everything together and get 6.

   The general algorithm for n dimensions and a vertex with coordinates
   \f$\vec{x}=(x_0,\ldots,x_{n-1})\f$ is as follows:
   - For each dimension d from 0 to n-1
    - Count the vertices in the n-d dimensional simplex of width
      \f$x_d-1\f$,
   - Add all counts together to get the index of the vertex

   In formulas it looks like this:

   <!--[ This is the more readable text version of the stuff below ]

   Let I(n, X) be the index of point X in the n-dimensional Kuhn0
   simplex.  X is a vector X=(x[0], ..., x[n-1]).  It measures the
   position of the point in gridunits and thus is integer.

            n-1                 n-1
            --		        --  ( n-i+x[i]-1 )
   I(n, X) = >  N(n-i, x[i]-1) = >   (	         )
            --		        --  (    n-i     )
            i=0		        i=0

   Since the dimensions within the Kuhn0 Simplex have a defined order
   (x[0] >= x[1] >= ... >= x[n-1]) they cannot simply be swapped so the
   sum is somewhat ugly.
   -->

   Let \f$I(n,\vec{x})\f$ be the index of point \f$\vec{x}\f$ in the
   n-dimensional Kuhn0 simplex.  The coordinates measure the position
   of the point in gridunits and thus are integer.

   \f[I(n,\vec{x})=\sum_{i=0}^{n-1}N(n-i,x_i-1)=\sum_{i=0}^{n-1}\left({n-i+x_i-1}\atop{n-i}\right)\f]

   Since the coordinates of a vertex within the Kuhn0 obey the relation
   \f$x_0\geq x_1\geq\ldots\geq x_{n-1}\f$, they cannot simply be
   swapped so the sum is somewhat ugly.

   @section Kuhn0SubelementIndexing Index of a subelement within a Kuhn0 simplex
   <!--======================================================================-->

   We don't know of a way to simply map a subelement of a Kuhn0 simplex
   to an index number.  Luckily, the iterator interface only requires
   that we be able to calculate the next subelement.

   Each subelement is a Kuhn (sub)simplex which triangulates a
   hypercube.  We need to remember the vertex which is the origin of
   that hypercube and the index of the permutation of that identifies
   the Kuhn subsimplex.  Now to get to the next subelement, we simply
   need to increment the permutation index, and if the overflows we
   reset it and increment the origin to the next vertex (we already
   know how to do that).

   Some subelements generated this way are outside the refined Kuhn0
   simplex, so we have to check for that, and skip them.

   @section PermutationIndexing Index of a permutation
   <!--============================================-->

   We need to index the n! permutations of the integers from 0 to n-1
   and a way to calculate the permutation if given the index.

   I'll discuss the permutation P, which operates on a vector
   \f$\vec{x}=(x_0,\ldots,x_{n-1})\f$.

   P can be made up of n transpositions, \f$P=T_0\cdots T_{n-1}\f$.
   Each transposition \f$T_i\f$ exchanges some arbitrary element
   \f$x_{t_i}\f$ with the element \f$x_i\f$, where \f$t_i\leq i\f$.  So
   we can totally describe \f$T_i\f$ by the integer \f$t_i\f$.  Thus we
   can describe P by the integer vector
   \f$\vec{t}=(t_0,\cdots,t_{n-1})\f$, where \f$0\leq t_i\leq i\f$.

   Now we need to encode the vector \f$\vec{t}\f$ into a single number.
   To do that we view \f$t_i\f$ as digit i of a number p written in a
   <em>base faculty notation</em>:

   \f[p=\sum_{i=1}^{n-1}i!t_i\f]

   This number p is unique for each possible permutation P so we could
   use this as index.  There is a problem though: we would like the
   identity permutation \f$\vec{x}=P\vec{x}\f$ to have index 0.  So we
   define the index I of the permutation slightly differently:

   \f[I=\sum_{i=1}^{n-1}i!(i-t_i)\f]

   I can easily calculate the \f$t_i\f$ from I:

   \f[i-t_i=(I/i!)\%(i+1)\f]
   ('/' is integer division and '%' calculates the remainder).

   @section KuhnToReference Mapping between some Kuhn and the reference simplex
   <!--=====================================================================-->

   @image html referencetokuhn0.png "Step 1 moves each point by its x2 value into x1 direction.  Step 2 moves each point by its new x1 value into x0 direction."

   The algorithm to transform a point
   \f$\vec{x}=(x_0,\ldots,x_{n-1})\f$ from the reference simplex of
   dimension n to the Kuhn0 simplex (as illustrated in the image) is as
   follows:
   - For each dimension d from n-2 down to 0:
    - \f$x_d:=x_d+x_{d+1}\f$.

   The reverse (from Kuhn0 to reference) is simple as well:
   - For each dimension d from 0 up to n-2:
    - \f$x_d:=x_d-x_{d+1}\f$.

   @par Arbitrary Kuhn simplices
   <!-------------------------->

   For arbitrary Kuhn simplices we have to take the permutation of that
   simplex into account.  So to map from the reference simplex of n
   dimensions to the Kuhn simplex with the permutation P (which is
   described by the vector \f$\vec{p}=P(0,\ldots,n-1)\f$) we do:
   - For each dimension d from n-2 down to 0:
    - \f$x_{p_d}:=x_{p_d}+x_{p_{d+1}}\f$.

   And or the reverse:
   - For each dimension d from 0 up to n-2:
    - \f$x_{p_d}:=x_{p_d}-x_{p_{d+1}}\f$.
 */

#include "../../../common/geometrytype.hh"
#include "../../../common/fvector.hh"
#include "../../../common/helpertemplates.hh"
#include "base.cc"

namespace Dune {

  namespace RefinementImp {

    /*! @brief This namespace contains the @ref Refinement
               implementation for simplices (triangles,
               tetrahedrons...)

       See @ref SimplexRefinement.
     */
    namespace Simplex {

      // //////////////////
      //
      //! @name Utilities
      //

      //@{

      /*! @brief Calculate n!

         Runtime is of order O(n).
       */
      int factorial(int n)
      {
        int prod = 1;
        for(int i = 1; i <= n; ++i)
          prod *= i;
        return prod;
      }

      /*! @brief Calculate factorial at compile time

         @param n Number to take the factorial from
       */
      template<int n> struct Factorial {
        enum {
          //! Value of the factorial @hideinitializer
          value = Factorial<n-1>::value * n
        };
      };
      template<> struct Factorial<0>
      { enum { value = 1 }; };

      /*! @brief calculate \f$\left({upper}\atop{lower}\right)\f$

         Runtime is of order O(min {lower, upper-lower})
       */
      int binomial(int upper, int lower)
      {
        if(lower > upper - lower)
          lower = upper - lower;
        if(lower < 0)
          return 0;
        int prod = 1;
        for(int i = upper - lower + 1; i <= upper; ++i)
          prod *= i;
        return prod / factorial(lower);
      }

      /*! @brief calculate the index of a given gridpoint within a
                 Kuhn0 simplex

         Runtime is of order O(dimension^2) (or better for dimension >
         the coordinates of the point)
       */
      template<int dimension>
      int pointIndex(const FieldVector<int, dimension> &point)
      {
        int index = 0;
        for(int i = 0; i < dimension; ++i)
          index += binomial(dimension-i + point[i]-1, dimension-i);
        return index;
      }

      /*! @brief Calculate permutation from it's index

         Runtime is of order O(n).
       */
      template<int n>
      FieldVector<int, n> getPermutation(int m)
      {
        FieldVector<int, n> perm;
        for(int i = 0; i < n; ++i)
          perm[i] = i;

        int base = 1;
        for(int i = 1; i <= n; ++i)
          base *= i;

        for(int i = n; i > 0; --i) {
          base /= i;
          int d = m / base;
          m %= base;
          int t = perm[i-1]; perm[i-1] = perm[i-1-d]; perm[i-1-d] = t;
        }
        return perm;
      }

#if 0
      Has to be checked
      // calculate the index of a permutation
      template<int n>
      int getPermIndex(const FieldVector<int, n>& test) // O(n^2)
      {
        int m = 0;
        FieldVector<int, n> perm;
        for(int i = 0; i < n; ++i)
          perm[i] = i;

        int base = 1;
        for(int i = 1; i <= n; ++i)
          base *= i;

        for(int i = n; i > 0; --i) {
          base /= i;
          int d;
          for(d = 0; d < i; ++d)
            if(test[i-1] == perm[i-1-d])
              break;
          m += d * base;
          int d = m / base;
          m %= base;
          perm[i-1-d] = perm[i-1];
        }
      }
#endif

      // map between the reference simplex and some arbitrary kuhn simplex (denoted by it's permutation)
      /*! @brief Map from the reference simplex to some Kuhn simplex

         @param dimension Dimension of the simplices

         Runtime is of order O(dimension)
       */
      template<int dimension, class CoordType>
      FieldVector<CoordType, dimension>
      referenceToKuhn( //! Point to map
        FieldVector<CoordType, dimension> point,
        //! Permutation of the Kuhn simplex to map to
        const FieldVector<int, dimension> &kuhn)
      {
        for(int i = dimension - 1; i > 0; --i)
          point[kuhn[i-1]] += point[kuhn[i]];
        return point;
      }

      /*! @brief Map from some Kuhn simplex to the reference simplex

         @param dimension Dimension of the simplices

         Runtime is of order O(dimension)
       */
      template<int dimension, class CoordType>
      FieldVector<CoordType, dimension>
      kuhnToReference( //! Point to map
        FieldVector<CoordType, dimension> point,
        //! Permutation of the Kuhn simplex to map from
        const FieldVector<int, dimension> &kuhn)
      {
        for(int i = 0; i < dimension - 1; ++i)
          point[kuhn[i]] -= point[kuhn[i+1]];
        return point;
      }


      //@} <!-- Group utilities -->

      // /////////////////////////////////////////
      //
      // refinement implementation for simplices
      //

      template<int mydimension, int coorddimension, class GridImp>
      class Geometry;

      template<int dimension_, class CoordType>
      class RefinementImp
      {
      public:
        enum { dimension = dimension_ };
        // to make Dune::Geometry work:
        struct GridFamily;
        typedef CoordType ctype;
        enum { dimensionworld = dimension };

        template<int codimension>
        struct Codim;
        typedef typename Codim<dimension>::SubEntityIterator VertexIterator;
        typedef FieldVector<CoordType, dimension> CoordVector;
        typedef typename Codim<0>::SubEntityIterator ElementIterator;
        typedef FieldVector<int, dimension+1> IndexVector;

        static int nVertices(int level);
        static VertexIterator vBegin(int level);
        static VertexIterator vEnd(int level);

        static int nElements(int level);
        static ElementIterator eBegin(int level);
        static ElementIterator eEnd(int level);
      };

      template<int dimension, class CoordType>
      template<int codimension>
      struct RefinementImp<dimension, CoordType>::Codim
      {
        class SubEntityIterator;
        typedef Dune::Geometry<dimension-codimension, dimension, RefinementImp<dimension, CoordType>, Geometry> Geometry;
      };

      template<int dimension, class CoordType>
      int
      RefinementImp<dimension, CoordType>::
      nVertices(int level)
      {
        return binomial(dimension + (1 << level), dimension);
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
        return 1 << (level * dimension);
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

      template<int dimension, class CoordType, int codimension>
      class RefinementIteratorSpecial;

      // vertices

      template<int dimension, class CoordType>
      class RefinementIteratorSpecial<dimension, CoordType, dimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;
        typedef typename Refinement::CoordVector CoordVector;
        typedef RefinementIteratorSpecial<dimension, CoordType, dimension> This;

        RefinementIteratorSpecial(int level, bool end = false);

        void increment();
        bool equals(const This &other) const;

        CoordVector coords() const;
        int index() const;
      protected:
        typedef FieldVector<int, dimension> Vertex;

        int size;
        Vertex vertex;
      };

      template<int dimension, class CoordType>
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      RefinementIteratorSpecial(int level, bool end)
        : size(1<<level)
      {
        if(end)
          vertex[0] = size + 1;
        else
          vertex[0] = 0;
        for(int i = 1; i < dimension; ++ i)
          vertex[i] = 0;
      }

      template<int dimension, class CoordType>
      void
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      increment()
      {
        assert(vertex[0] <= size);
        for(int i = dimension - 1; i >= 0; --i) {
          ++vertex[i];
          if(i == 0 || vertex[i] <= vertex[i-1])
            break;
          else
            vertex[i] = 0;
        }
      }

      template<int dimension, class CoordType>
      bool
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      equals(const This &other) const
      {
        return size == other.size && vertex == other.vertex;
      }

      template<int dimension, class CoordType>
      typename RefinementIteratorSpecial<dimension, CoordType, dimension>::CoordVector
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      coords() const
      {
        Vertex ref = kuhnToReference(vertex, getPermutation<dimension>(0));

        CoordVector coords;
        for(int i = 0; i < dimension; ++i)
          coords[i] = CoordType(ref[i]) / size;
        return coords;
      }

      template<int dimension, class CoordType>
      int
      RefinementIteratorSpecial<dimension, CoordType, dimension>::
      index() const
      {
        return pointIndex(vertex);
      }

      // elements

      template<int mydimension, class GridImp>
      class MakeableGeometry;

      template<int dimension, class CoordType>
      class RefinementIteratorSpecial<dimension, CoordType, 0>
      {
      public:
        typedef RefinementImp<dimension, CoordType> RefinementImp;
        typedef typename RefinementImp::IndexVector IndexVector;
        typedef typename RefinementImp::template Codim<0>::Geometry Geometry;
        typedef RefinementIteratorSpecial<dimension, CoordType, 0> This;

        RefinementIteratorSpecial(int level, bool end = false);

        void increment();
        bool equals(const This &other) const;

        IndexVector vertexIndices() const;
        int index() const;
        const Geometry &geometry() const;
      protected:
        typedef FieldVector<int, dimension> Vertex;
        enum { nKuhnSimplices = Factorial<dimension>::value };

        Vertex origin;
        int kuhnIndex;
        int size;
        int index_;
      private:
        mutable bool builtGeometry;
        mutable MakeableGeometry<dimension, RefinementImp> geometry_;
      };

      template<int dimension, class CoordType>
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      RefinementIteratorSpecial(int level, bool end)
        : kuhnIndex(0), size(1<<level), index_(0),
          builtGeometry(false), geometry_(level)
      {
        for(int i = 0; i < dimension; ++i)
          origin[i] = 0;
        if(end) {
          index_ = RefinementImp::nElements(level);
          origin[0] = size;
        }
      }

      template<int dimension, class CoordType>
      void
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      increment()
      {
        assert(origin[0] < size);

        ++index_;
        builtGeometry = false;

        while(1) {
          ++kuhnIndex;
          if(kuhnIndex == nKuhnSimplices) {
            kuhnIndex = 0;
            // increment origin
            for(int i = dimension - 1; i >= 0; --i) {
              ++origin[i];
              if(i == 0 || origin[i] <= origin[i-1])
                break;
              else
                origin[i] = 0;
            }
          }

          // test whether the current simplex has any corner outside the kuhn0 simplex
          FieldVector<int, dimension> perm = getPermutation<dimension>(kuhnIndex);
          Vertex corner = origin;
          bool outside = false;
          for(int i = 0; i < dimension; ++i) {
            // next corner
            ++corner[perm[i]];
            if(perm[i] > 0)
              if(corner[perm[i]] > corner[perm[i]-1]) {
                outside = true;
                break;
              }
          }
          if(!outside)
            return;
        }
      }

      template<int dimension, class CoordType>
      bool
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      equals(const This &other) const
      {
        return size == other.size && index_ == other.index_;
      }

      template<int dimension, class CoordType>
      typename RefinementIteratorSpecial<dimension, CoordType, 0>::IndexVector
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      vertexIndices() const
      {
        IndexVector indices;
        FieldVector<int, dimension> perm = getPermutation<dimension>(kuhnIndex);
        Vertex vertex = origin;
        indices[0] = pointIndex(vertex);
        for(int i = 0; i < dimension; ++i) {
          ++vertex[perm[i]];
          indices[i+1] = pointIndex(vertex);
        }
        if (kuhnIndex%2 == 1)
          for(int i = 0; i < (dimension+1)/2; ++i) {
            int t = indices[i];
            indices[i] = indices[dimension-i];
            indices[dimension-i] = t;
          }
        return indices;
      }

      template<int dimension, class CoordType>
      int
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      index() const
      {
        return index_;
      }

      template<int dimension, class CoordType>
      const typename RefinementIteratorSpecial<dimension, CoordType, 0>::Geometry &
      RefinementIteratorSpecial<dimension, CoordType, 0>::
      geometry() const
      {
        if(!builtGeometry) {
          geometry_.make(origin, kuhnIndex);
          builtGeometry = true;
        }

        return geometry_;
      }

      // common

      template<int dimension, class CoordType>
      template<int codimension>
      class RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator
        : public ForwardIteratorFacade<typename RefinementImp<dimension, CoordType>::template Codim<codimension>::SubEntityIterator, int>,
          public RefinementIteratorSpecial<dimension, CoordType, codimension>
      {
      public:
        typedef RefinementImp<dimension, CoordType> Refinement;

        SubEntityIterator(int level, bool end = false);
      };

      template<int dimension, class CoordType>
      template<int codimension>
      RefinementImp<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
      SubEntityIterator(int level, bool end)
        : RefinementIteratorSpecial<dimension, CoordType, codimension>(level, end)
      {}

      // ///////////////
      //
      //  The Geometry
      //

      template<int mydimension, class GridImp>
      class ReferenceGeometryInstance;

      template<int mydimension, int coorddimension, class GridImp>
      class Geometry : public GeometryDefaultImplementation<mydimension, coorddimension, GridImp, Geometry>
      {
        typedef typename GridImp::ctype ct;
        typedef Dune::Geometry<mydimension, mydimension, GridImp, Simplex::Geometry> ReferenceGeometry;
      public:
        GeometryType type() const
        { return GeometryType(GeometryType::simplex, mydimension); }

        int corners() const
        { return mydimension + 1; }

        const FieldVector<ct, coorddimension>& operator[] (int i) const
        {
          if(!builtCoords) {
            FieldVector<ct, coorddimension> v = 0;
            FieldVector<int, mydimension> perm = getPermutation<mydimension>(kuhnIndex);
            for(int i = 0; i < mydimension; ++i) {
              coords[i] = global(v);
              v[perm[i]] += 1;
            }
            coords[mydimension] = global(v);
            builtCoords = true;
          }
          return coords[i];
        }

        static const ReferenceGeometry& refelem()
        { return ReferenceGeometryInstance<mydimension, GridImp>::instance(); }

        FieldVector<ct, coorddimension> global(const FieldVector<ct, mydimension>& local) const
        {
          FieldVector<ct, mydimension> v = referenceToKuhn(local, getPermutation<mydimension>(kuhnIndex));
          v += origin;
          v /= (1<<level);
          return kuhnToReference(v, getPermutation<mydimension>(0));
        }

        FieldVector<ct, mydimension> local(const FieldVector<ct, coorddimension>& global) const
        {
          FieldVector<ct, mydimension> v = referenceToKuhn(global, getPermutation<mydimension>(0));
          v *= (1<<level);
          v -= origin;
          return kuhnToReference(v, getPermutation<mydimension>(kuhnIndex));
        }

        bool checkInside (const FieldVector<ct, mydimension>& local) const
        {
          for(int i = 0; i < mydimension; ++i)
            if(local[i] < 0)
              return false;

          ct sum = 0;
          for(int i = 0; i < mydimension; ++i)
            sum += local[i];
          return sum <= 1;
        }

        ct integrationElement(const FieldVector<ct, mydimension>& local) const
        {
          return ct(1) / RefinementImp<mydimension, ct>::nElements(level);
        }

        const FieldMatrix<ct, mydimension, mydimension>& jacobianInverse(const FieldVector<ct, mydimension>& local) const
        {
          if(!builtJinv) {
            // create unit vectors
            FieldMatrix<int, mydimension, mydimension> M = 0;
            for(int i = 0; i < mydimension; ++i)
              M[i][i] = 1;
            // transform them into local coordinates
            for(int i = 0; i < mydimension; ++i)
              M[i] = kuhnToReference(referenceToKuhn(M[i], getPermutation<mydimension>(0)), getPermutation<mydimension>(kuhnIndex));
            // scale by the inverse size of the element and transpose the matrix
            for(int i = 0; i < mydimension; ++i)
              for(int j = 0; j < mydimension; ++j)
                Jinv[i][j] = ct(1<<level) * M[j][i];

            builtJinv = true;
          }

          return Jinv;
        }

        Geometry(int level_)
          : coords(), builtCoords(false), Jinv(), builtJinv(false),
            level(level_), kuhnIndex(0), origin()
        {
          IsTrue<mydimension == coorddimension>::yes();
        }

        void make(const FieldVector<int, coorddimension> &origin_, int kuhnIndex_)
        {
          origin = origin_;
          kuhnIndex = kuhnIndex_;
          builtCoords = false;
          builtJinv = false;
        }
      private:
        mutable FieldVector<FieldVector<ct, coorddimension>, mydimension+1> coords;
        mutable bool builtCoords;
        mutable FieldMatrix<ct, mydimension, mydimension> Jinv;
        mutable bool builtJinv;
        int level;
        int kuhnIndex;
        FieldVector<int, coorddimension> origin;
      };

      template<int mydimension, class GridImp>
      class MakeableGeometry : public Dune::Geometry<mydimension, mydimension, GridImp, Geometry>
      {
      public:
        MakeableGeometry(int level)
          : Dune::Geometry<mydimension, mydimension, GridImp, Geometry>(Geometry<mydimension, mydimension, GridImp>(level))
        {}

        void make(const FieldVector<int, mydimension> &origin, int kuhnIndex)
        { realGeometry.make(origin, kuhnIndex); }
      private:
        using Dune::Geometry<mydimension, mydimension, GridImp, Geometry>::realGeometry;
      };

      template<int mydimension, class GridImp>
      class ReferenceGeometryInstance
      {
        typedef Dune::Geometry<mydimension, mydimension, GridImp, Geometry> ReferenceGeometry;
      public:
        static ReferenceGeometry &instance()
        {
          if(instance_ == 0)
            instance = new ReferenceGeometry(Geometry<mydimension, mydimension, GridImp>(0));
          return *instance;
        }
      private:
        static ReferenceGeometry *instance_;
      };
      template<int mydimension, class GridImp>
      Dune::Geometry<mydimension, mydimension, GridImp, Geometry> *ReferenceGeometryInstance<mydimension, GridImp>::instance_ = 0;

    } // namespace Simplex

    // ///////////////////////
    //
    // The refinement traits
    //

    template<class CoordType, int dim>
    struct Traits<GeometryType::simplex, CoordType, GeometryType::simplex, dim>
    {
      typedef Simplex::RefinementImp<dim, CoordType> Imp;
    };

  } // namespace RefinementImp

} // namespace Dune

#endif //DUNE_GRID_COMMON_REFINEMENT_SIMPLEX_CC
