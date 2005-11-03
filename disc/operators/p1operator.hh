// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1OPERATOR_HH
#define DUNE_P1OPERATOR_HH

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/mcmgmapper.hh"
#include "istl/bvector.hh"
#include "istl/operators.hh"
#include "istl/bcrsmatrix.hh"

/**
 * @file
 * @brief  defines a class for piecewise linear finite element functions
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief defines a class for piecewise linear finite element functions
   *
   */

  // template meta program for inserting indices
  template<int n, int c>
  struct P1FEOperator_meta {
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addrowscube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                             const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      for (int i=0; i<refelem.size(c); i++)     // loop over subentities of codim c of e
      {
        int index = allmapper.template map<c>(e,i);
        if (!visited[index])
        {
          int corners = refelem.size(i,c,n);
          for (int j=0; j<corners/2; j++)                 // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
          {
            int alpha = vertexmapper.template map<n>(e,refelem.subEntity(i,c,j,n));
            int beta = vertexmapper.template map<n>(e,refelem.subEntity(i,c,corners-1-j,n));
            A.incrementrowsize(alpha);
            A.incrementrowsize(beta);
          }
          visited[index] = true;
        }
      }
      P1FEOperator_meta<n,c-1>::addrowscube(e,vertexmapper,allmapper,refelem,A,visited);
      return;
    }
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addindicescube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                                const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      for (int i=0; i<refelem.size(c); i++)
      {
        int index = allmapper.template map<c>(e,i);
        if (!visited[index])
        {
          int corners = refelem.size(i,c,n);
          for (int j=0; j<corners/2; j++)                 // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
          {
            int alpha = vertexmapper.template map<n>(e,refelem.subEntity(i,c,j,n));
            int beta = vertexmapper.template map<n>(e,refelem.subEntity(i,c,corners-1-j,n));
            A.addindex(alpha,beta);
            A.addindex(beta,alpha);
            //                            std::cout << "adding (" << alpha << "," << beta << ")" << std::endl;
            //                            std::cout << "adding (" << beta << "," << alpha << ")" << std::endl;
          }
          visited[index] = true;
        }
      }
      P1FEOperator_meta<n,c-1>::addindicescube(e,vertexmapper,allmapper,refelem,A,visited);
      return;
    }
  };
  template<int n>
  struct P1FEOperator_meta<n,0> {
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addrowscube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                             const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      int corners = refelem.size(n);
      for (int j=0; j<corners/2; j++)     // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
      {
        int alpha = vertexmapper.template map<n>(e,refelem.subEntity(0,0,j,n));
        int beta = vertexmapper.template map<n>(e,refelem.subEntity(0,0,corners-1-j,n));
        A.incrementrowsize(alpha);
        A.incrementrowsize(beta);
      }
      return;
    }
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addindicescube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                                const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      int corners = refelem.size(n);
      for (int j=0; j<corners/2; j++)     // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
      {
        int alpha = vertexmapper.template map<n>(e,refelem.subEntity(0,0,j,n));
        int beta = vertexmapper.template map<n>(e,refelem.subEntity(0,0,corners-1-j,n));
        A.addindex(alpha,beta);
        A.addindex(beta,alpha);
        //                                std::cout << "adding (" << alpha << "," << beta << ")" << std::endl;
        //                                std::cout << "adding (" << beta << "," << alpha << ")" << std::endl;
      }
      return;
    }
  };


  //! a class for mapping a P1 function to a P1 function
  template<class G, class RT, class IS>
  class AssembledP1FEOperator
  {
    typedef typename G::ctype DT;
    enum {n=G::dimension};
    typedef typename G::template Codim<0>::Entity Entity;
    typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
    typedef typename G::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;

    // a function to compute the number of nonzeros
    // does not work for prisms and pyramids yet ?!
    int nnz (const IS& is)
    {
      int s = 0;
      s += is.size(n);       // vertices
      //	  std::cout << "nnz vertices " << g.size(n) << std::endl;

      s += 2*is.size(n-1);     // edges
      //	  std::cout << "nnz edges " << g.size(n-1) << std::endl;

      for (int c=0; c<n-1; c++)
      {
        s += 2*is.size(c,cube)*(1<<(n-c-1));
        //		  std::cout << "nnz cubes codim " << c << " is " << g.size(c,cube) << std::endl;
      }

      return s;
    }

    // mapper: one data element per vertex
    template<int dim>
    struct P1Layout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        if (codim==dim) return true;
        return false;
      }
    };

    // mapper: one data element in every entity
    template<int dim>
    struct AllLayout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        return true;
      }
    };


  public:
    // export type used to store the matrix
    typedef BCRSMatrix<FieldMatrix<RT,1,1> > RepresentationType;

    AssembledP1FEOperator (const G& g, const IS& indexset)
      : grid(g),is(indexset),A(g.size(n),g.size(n),nnz(indexset),RepresentationType::random),
        vertexmapper(g,indexset),allmapper(g,indexset)
    {
      std::cout << "making " << g.size(n) << "x" << g.size(n) << " matrix with " << nnz(indexset) << " nonzeros" << std::endl;
      // set size of all rows to zero
      for (int i=0; i<g.size(n); i++)
        A.setrowsize(i,0);

      // build needs a flag for all entities of all codims
      std::vector<bool> visited(allmapper.size());
      for (int i=0; i<allmapper.size(); i++) visited[i] = false;

      // resize the S vector needed for detecting hanging nodes
      std::vector<unsigned char> S(vertexmapper.size());
      hanging.resize(vertexmapper.size());
      for (int i=0; i<vertexmapper.size(); i++)
      {
        S[i] = 100;           // the number of levels never exceeds 100 ...
        hanging[i] = false;
      }

      // LOOP I : handle each element and compute row sizes
      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::ReferenceElementContainer<DT,n>::value_type&
        refelem = ReferenceElements<DT,n>::general(gt);

        // vertices, c=n
        for (int i=0; i<refelem.size(n); i++)
        {
          int index = allmapper.template map<n>(*it,i);
          int alpha = vertexmapper.template map<n>(*it,i);
          if (!visited[index])
          {
            A.incrementrowsize(alpha);
            visited[index] = true;
            //				  printf("increment row %04d\n",alpha);
            //				  std::cout << "increment row " << alpha << std::endl;
          }
          if (S[alpha]>it->level()) S[alpha] = it->level();                 // compute minimum
        }

        // edges for all element types, c=n-1
        for (int i=0; i<refelem.size(n-1); i++)
        {
          int index = allmapper.template map<n-1>(*it,i);
          int alpha = vertexmapper.template map<n>(*it,refelem.subEntity(i,n-1,0,n));
          int beta = vertexmapper.template map<n>(*it,refelem.subEntity(i,n-1,1,n));
          if (!visited[index])
          {
            A.incrementrowsize(alpha);
            A.incrementrowsize(beta);
            visited[index] = true;
            //				  printf("increment row %04d\n",alpha);
            //				  printf("increment row %04d\n",beta);
            //				  std::cout << "increment row " << alpha << std::endl;
            //				  std::cout << "increment row " << beta << std::endl;
          }
        }

        // for codim n-2 to 0 we need a template metaprogram
        if (gt==Dune::cube)
          P1FEOperator_meta<n,n-2>::addrowscube(*it,vertexmapper,allmapper,refelem,A,visited);
      }

      // now the row sizes have been set
      A.endrowsizes();

      // clear the flags for the next round
      for (int i=0; i<allmapper.size(); i++) visited[i] = false;
      std::cout << "allmapper has size " << allmapper.size() << std::endl;
      std::cout << "vertexmapper has size " << vertexmapper.size() << std::endl;

      // LOOP II : handle each leaf element and insert the nonzeros
      //	  eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::ReferenceElementContainer<DT,n>::value_type&
        refelem = ReferenceElements<DT,n>::general(gt);
        //                std::cout << "ELEM " << GeometryName(gt) << std::endl;
        // vertices, c=n
        for (int i=0; i<refelem.size(n); i++)
        {
          int index = allmapper.template map<n>(*it,i);
          //                      std::cout << "vertex allindex " << index << std::endl;
          if (!visited[index])
          {
            int alpha = vertexmapper.template map<n>(*it,i);
            A.addindex(alpha,alpha);
            visited[index] = true;
            //				  printf("adding (%04d,%04d) index=%04d\n",alpha,alpha,index);
            //				  std::cout << "adding (" << alpha << "," << alpha << ")" << std::endl;
          }
        }

        // edges for all element types, c=n-1
        for (int i=0; i<refelem.size(n-1); i++)
        {
          int index = allmapper.template map<n-1>(*it,i);
          //                      std::cout << "edge allindex " << index << std::endl;
          if (!visited[index])
          {
            int alpha = vertexmapper.template map<n>(*it,refelem.subEntity(i,n-1,0,n));
            int beta = vertexmapper.template map<n>(*it,refelem.subEntity(i,n-1,1,n));
            A.addindex(alpha,beta);
            A.addindex(beta,alpha);
            visited[index] = true;
            //				  printf("adding (%04d,%04d) index=%04d\n",alpha,beta,index);
            //				  printf("adding (%04d,%04d) index=%04d\n",beta,alpha,index);
            //                            std::cout << "adding (" << alpha << "," << beta << ")" << std::endl;
            //                            std::cout << "adding (" << beta << "," << alpha << ")" << std::endl;
          }
        }

        // for codim n-2 to 0 we need a template metaprogram
        if (gt==Dune::cube)
          P1FEOperator_meta<n,n-2>::addindicescube(*it,vertexmapper,allmapper,refelem,A,visited);

        // detect hanging nodes
        IntersectionIterator endiit = it->iend();
        for (IntersectionIterator iit = it->ibegin(); iit!=endiit; ++iit)
          if (iit.neighbor())
          {
            // check if neighbor is on lower level
            const EEntityPointer outside = iit.outside();
            if (it->level()<=outside->level()) continue;

            // loop over all vertices of this face
            for (int j=0; j<refelem.size(iit.numberInSelf(),1,n); j++)
            {
              int alpha = vertexmapper.template map<n>(*it,refelem.subEntity(iit.numberInSelf(),1,j,n));
              if (S[alpha]==it->level()) hanging[alpha] = true;
            }
          }
      }

      // now the matrix is ready for use
      A.endindices();

      // count hanging nodes
      int hangingnodes = 0;
      for (int i=0; i<vertexmapper.size(); i++)
        if (hanging[i]) hangingnodes++;

      std::cout << "matrix initialized: " << hangingnodes << " hanging nodes detected" << std::endl;
    }

    //! return const reference to coefficient vector
    const RepresentationType& operator* () const
    {
      return A;
    }

    //! return reference to coefficient vector
    RepresentationType& operator* ()
    {
      return A;
    }

  protected:
    const G& grid;
    const IS& is;
    RepresentationType A;
    MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout> vertexmapper;
    MultipleCodimMultipleGeomTypeMapper<G,IS,AllLayout> allmapper;
    std::vector<bool> hanging;
  };


  template<class G, class RT>
  class LeafAssembledP1FEOperator : public AssembledP1FEOperator<G,RT,typename G::Traits::LeafIndexSet>
  {
  public:
    LeafAssembledP1FEOperator (const G& grid)
      : AssembledP1FEOperator<G,RT,typename G::Traits::LeafIndexSet>(grid,grid.leafIndexSet())
    {}
  };


  template<class G, class RT>
  class LevelAssembledP1FEOperator : public AssembledP1FEOperator<G,RT,typename G::Traits::LevelIndexSet>
  {
  public:
    LevelAssembledP1FEOperator (const G& grid, int level)
      : AssembledP1FEOperator<G,RT,typename G::Traits::LevelIndexSet>(grid,grid.levelIndexSet(level))
    {}
  };


  /** @} */

}
#endif
