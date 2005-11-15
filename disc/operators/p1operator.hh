// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1OPERATOR_HH
#define DUNE_P1OPERATOR_HH

#include <iostream>
#include <vector>
#include <set>
#include <map>
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

  struct P1FEOperatorLink
  {
    int first,second;
    P1FEOperatorLink (int a, int b) : first(a),second(b) {}
    bool operator< (const P1FEOperatorLink& x) const
    {
      if (first<x.first) return true;
      if (first==x.first && second<x.second) return true;
      return false;
    }
    bool operator== (const P1FEOperatorLink& x) const
    {
      if (first==x.first && second==x.second) return true;
      return false;
    }
  };

  // template meta program for inserting indices
  template<int n, int c>
  struct P1FEOperator_meta {
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addrowscube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                             const Refelem& refelem, Matrix& A, std::vector<bool>& visited,
                             int hangingnodes, std::set<P1FEOperatorLink>& links)
    {
      if (refelem.type(0,0)==Dune::cube)
      {
        for (int i=0; i<refelem.size(c); i++)           // loop over subentities of codim c of e
        {
          int index = allmapper.template map<c>(e,i);
          if (!visited[index])
          {
            int corners = refelem.size(i,c,n);
            for (int j=0; j<corners/2; j++)                       // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
            {
              int alpha = vertexmapper.template map<n>(e,refelem.subEntity(i,c,j,n));
              int beta = vertexmapper.template map<n>(e,refelem.subEntity(i,c,corners-1-j,n));
              A.incrementrowsize(alpha);
              A.incrementrowsize(beta);
              if (hangingnodes>0)                             // delete standard links
              {
                links.erase(P1FEOperatorLink(alpha,beta));
                links.erase(P1FEOperatorLink(beta,alpha));
              }
              //                                  printf("increment row %04d\n",alpha);
              //                                  printf("increment row %04d\n",beta);
            }
            visited[index] = true;
          }
        }
      }
      if (refelem.type(0,0)==Dune::pyramid && c==1)
      {
        int index = allmapper.template map<c>(e,0);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,2);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          alpha = vertexmapper.template map<n>(e,1);
          beta = vertexmapper.template map<n>(e,3);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          visited[index] = true;
        }
      }
      if (refelem.type(0,0)==Dune::prism && c==1)
      {
        int index = allmapper.template map<c>(e,1);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,4);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          alpha = vertexmapper.template map<n>(e,1);
          beta = vertexmapper.template map<n>(e,3);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          visited[index] = true;
        }
        index = allmapper.template map<c>(e,2);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,1);
          int beta = vertexmapper.template map<n>(e,5);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          alpha = vertexmapper.template map<n>(e,2);
          beta = vertexmapper.template map<n>(e,4);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          visited[index] = true;
        }
        index = allmapper.template map<c>(e,3);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,5);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          alpha = vertexmapper.template map<n>(e,2);
          beta = vertexmapper.template map<n>(e,3);
          A.incrementrowsize(alpha);
          A.incrementrowsize(beta);
          if (hangingnodes>0)                 // delete standard links
          {
            links.erase(P1FEOperatorLink(alpha,beta));
            links.erase(P1FEOperatorLink(beta,alpha));
          }
          visited[index] = true;
        }
      }
      P1FEOperator_meta<n,c-1>::addrowscube(e,vertexmapper,allmapper,refelem,A,visited,hangingnodes,links);
      return;
    }
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addindicescube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                                const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      if (refelem.type(0,0)==Dune::cube)
      {
        for (int i=0; i<refelem.size(c); i++)
        {
          int index = allmapper.template map<c>(e,i);
          if (!visited[index])
          {
            int corners = refelem.size(i,c,n);
            for (int j=0; j<corners/2; j++)                       // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
            {
              int alpha = vertexmapper.template map<n>(e,refelem.subEntity(i,c,j,n));
              int beta = vertexmapper.template map<n>(e,refelem.subEntity(i,c,corners-1-j,n));
              A.addindex(alpha,beta);
              A.addindex(beta,alpha);
              //                                  printf("adding (%04d,%04d) index=%04d\n",alpha,beta,index);
              //                                  printf("adding (%04d,%04d) index=%04d\n",beta,alpha,index);
            }
            visited[index] = true;
          }
        }
      }
      if (refelem.type(0,0)==Dune::pyramid && c==1)
      {
        int index = allmapper.template map<c>(e,0);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,2);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          alpha = vertexmapper.template map<n>(e,1);
          beta = vertexmapper.template map<n>(e,3);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          visited[index] = true;
        }
      }
      if (refelem.type(0,0)==Dune::prism && c==1)
      {
        int index = allmapper.template map<c>(e,1);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,4);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          alpha = vertexmapper.template map<n>(e,1);
          beta = vertexmapper.template map<n>(e,3);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          visited[index] = true;
        }
        index = allmapper.template map<c>(e,2);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,1);
          int beta = vertexmapper.template map<n>(e,5);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          alpha = vertexmapper.template map<n>(e,2);
          beta = vertexmapper.template map<n>(e,4);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          visited[index] = true;
        }
        index = allmapper.template map<c>(e,3);
        if (!visited[index])
        {
          int alpha = vertexmapper.template map<n>(e,0);
          int beta = vertexmapper.template map<n>(e,5);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
          alpha = vertexmapper.template map<n>(e,2);
          beta = vertexmapper.template map<n>(e,3);
          A.addindex(alpha,beta);
          A.addindex(beta,alpha);
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
                             const Refelem& refelem, Matrix& A, std::vector<bool>& visited,
                             int hangingnodes, std::set<P1FEOperatorLink>& links)
    {
      if (refelem.type(0,0)!=Dune::cube) return;
      int corners = refelem.size(n);
      for (int j=0; j<corners/2; j++)     // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
      {
        int alpha = vertexmapper.template map<n>(e,refelem.subEntity(0,0,j,n));
        int beta = vertexmapper.template map<n>(e,refelem.subEntity(0,0,corners-1-j,n));
        A.incrementrowsize(alpha);
        A.incrementrowsize(beta);
        if (hangingnodes>0)           // delete standard links
        {
          links.erase(P1FEOperatorLink(alpha,beta));
          links.erase(P1FEOperatorLink(beta,alpha));
        }
        //                printf("increment row %04d\n",alpha);
        //                printf("increment row %04d\n",beta);
      }
      return;
    }
    template<class Entity, class VMapper, class AMapper, class Refelem, class Matrix>
    static void addindicescube (const Entity& e, const VMapper& vertexmapper, const AMapper& allmapper,
                                const Refelem& refelem, Matrix& A, std::vector<bool>& visited)
    {
      if (refelem.type(0,0)!=Dune::cube) return;
      int corners = refelem.size(n);
      for (int j=0; j<corners/2; j++)     // uses fact that diagonals are (0,corners-1), (1,corners-2) ...
      {
        int alpha = vertexmapper.template map<n>(e,refelem.subEntity(0,0,j,n));
        int beta = vertexmapper.template map<n>(e,refelem.subEntity(0,0,corners-1-j,n));
        A.addindex(alpha,beta);
        A.addindex(beta,alpha);
        //                printf("adding (%04d,%04d)\n",alpha,beta);
        //                printf("adding (%04d,%04d)\n",beta,alpha);
      }
      return;
    }
  };


  //! a class for mapping a P1 function to a P1 function
  template<class G, class RT, class IS, int m=1>
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

      // hanging node correction
      s += links.size();

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

    // extra initialization function
    // 1) determine hanging nodes as described in the paper
    // 2) generate a set with additional links
    //    The standard links are deleted later on
    bool init (const G& g, const IS& indexset)
    {
      std::cout << "calling init" << std::endl;

      // resize the S vector needed for detecting hanging nodes
      hanging.resize(vertexmapper.size());
      std::vector<unsigned char> S(vertexmapper.size());
      for (int i=0; i<vertexmapper.size(); i++)
      {
        S[i] = 100;           // the number of levels never exceeds 100 ...
        hanging[i] = false;
      }

      // LOOP 1 : Prepare hanging node detection
      Iterator eendit = indexset.template end<0,All_Partition>();
      for (Iterator it = indexset.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::ReferenceElementContainer<DT,n>::value_type&
        refelem = ReferenceElements<DT,n>::general(gt);

        // compute S value in vertex
        for (int i=0; i<refelem.size(n); i++)
        {
          int alpha = vertexmapper.template map<n>(*it,i);
          if (S[alpha]>it->level()) S[alpha] = it->level();                 // compute minimum
        }
      }

      // LOOP 2 : second stage of detecting hanging nodes
      for (Iterator it = indexset.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::ReferenceElementContainer<DT,n>::value_type&
        refelem = ReferenceElements<DT,n>::general(gt);

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
              if (S[alpha]==it->level())
                hanging[alpha] = true;
            }
          }
      }

      // local to global maps
      int l2g[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
      int fl2g[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];

      // LOOP 3 : determine additional links due to hanging nodes
      for (Iterator it = indexset.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::ReferenceElementContainer<DT,n>::value_type&
        refelem = ReferenceElements<DT,n>::general(gt);

        // build local to global map
        bool hasHangingNodes = false;           // flag set to true if this element has hanging nodes
        for (int i=0; i<refelem.size(n); i++)
        {
          l2g[i] = vertexmapper.template map<n>(*it,i);
          if (hanging[l2g[i]]) hasHangingNodes=true;
        }
        if (!hasHangingNodes) continue;

        // handle father element if hanging nodes were detected
        // get father element
        const EEntityPointer father = it->father();

        // build local to global map for father
        for (int i=0; i<refelem.size(n); i++)
          fl2g[i] = vertexmapper.template map<n>(*father,i);

        // a map that inverts l2g
        std::map<int,int> g2l;
        for (int i=0; i<refelem.size(n); i++)
          g2l[l2g[i]] = i;

        // connect all fine nodes to all coarse nodes
        for (int i=0; i<refelem.size(n); i++)           // nodes in *it
          for (int j=0; j<refelem.size(n); j++)               // nodes in *father
            if (g2l.find(fl2g[j])==g2l.end())
            {
              links.insert(P1FEOperatorLink(l2g[i],fl2g[j]));
              links.insert(P1FEOperatorLink(fl2g[j],l2g[i]));
              //                                  std::cout << "link" << " gi=" << l2g[i] << " gj=" << fl2g[j] << std::endl;
              //                                  std::cout << "link" << " gi=" << fl2g[j] << " gj=" << l2g[i] << std::endl;
            }
      }

      // Note: links contains now also connections that are standard.
      // So below we have throw out these connections again!

      // count hanging nodes, can be used whether grid has hanging nodes at all
      hangingnodes = 0;
      for (int i=0; i<vertexmapper.size(); i++)
        if (hanging[i]) hangingnodes++;
      std::cout << "hanging nodes=" << hangingnodes
                << " links=" << links.size()
                << std::endl;

      return true;
    }

  public:
    // export type used to store the matrix
    typedef FieldMatrix<RT,m,m> BlockType;
    typedef BCRSMatrix<BlockType> RepresentationType;

    AssembledP1FEOperator (const G& g, const IS& indexset)
      : grid(g),is(indexset),vertexmapper(g,indexset),allmapper(g,indexset),links(),
        initialized(init(g,indexset)),A(g.size(n),g.size(n),nnz(indexset),RepresentationType::random)

    {
      // be verbose
      std::cout << "making " << g.size(n) << "x" << g.size(n) << " matrix with " << nnz(indexset) << " nonzeros" << std::endl;
      std::cout << "allmapper has size " << allmapper.size() << std::endl;
      std::cout << "vertexmapper has size " << vertexmapper.size() << std::endl;

      // set size of all rows to zero
      for (int i=0; i<g.size(n); i++)
        A.setrowsize(i,0);

      // build needs a flag for all entities of all codims
      std::vector<bool> visited(allmapper.size());
      for (int i=0; i<allmapper.size(); i++) visited[i] = false;

      // LOOP 4 : Compute row sizes
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
            //                            printf("increment row %04d\n",alpha);
          }
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
            if (hangingnodes>0)                       // delete standard links
            {
              links.erase(P1FEOperatorLink(alpha,beta));
              links.erase(P1FEOperatorLink(beta,alpha));
            }
            //                            printf("increment row %04d\n",alpha);
            //                            printf("increment row %04d\n",beta);
          }
        }

        // for codim n-2 to 0 we need a template metaprogram
        if (gt!=Dune::simplex)
          P1FEOperator_meta<n,n-2>::addrowscube(*it,vertexmapper,allmapper,refelem,A,visited,hangingnodes,links);
      }

      // additional links due to hanging nodes
      std::cout << "now links=" << links.size() << std::endl;
      for (typename std::set<P1FEOperatorLink>::iterator i=links.begin(); i!=links.end(); ++i)
        A.incrementrowsize(i->first);

      // now the row sizes have been set
      A.endrowsizes();

      // clear the flags for the next round, actually that is not necessary because addindex takes care of this
      for (int i=0; i<allmapper.size(); i++) visited[i] = false;

      // LOOP 5 : insert the nonzeros
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
          int alpha = vertexmapper.template map<n>(*it,i);
          //                      std::cout << "vertex allindex " << index << std::endl;
          if (!visited[index])
          {
            A.addindex(alpha,alpha);
            visited[index] = true;
            //                            printf("adding (%04d,%04d) index=%04d\n",alpha,alpha,index);
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
            //                            printf("adding (%04d,%04d) index=%04d\n",alpha,beta,index);
            //                            printf("adding (%04d,%04d) index=%04d\n",beta,alpha,index);
          }
        }

        // for codim n-2 to 0 we need a template metaprogram
        if (gt!=Dune::simplex)
          P1FEOperator_meta<n,n-2>::addindicescube(*it,vertexmapper,allmapper,refelem,A,visited);
      }

      // additional links due to hanging nodes
      for (typename std::set<P1FEOperatorLink>::iterator i=links.begin(); i!=links.end(); ++i)
        A.addindex(i->first,i->second);

      // now the matrix is ready for use
      A.endindices();

      // delete additional links
      links.clear();

      std::cout << "matrix initialized" << std::endl;
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
    MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout> vertexmapper;
    MultipleCodimMultipleGeomTypeMapper<G,IS,AllLayout> allmapper;
    std::vector<bool> hanging;
    std::set<P1FEOperatorLink> links;
    int hangingnodes;
    bool initialized;
    RepresentationType A;
  };


  template<class G, class RT, int m=1>
  class LeafAssembledP1FEOperator : public AssembledP1FEOperator<G,RT,typename G::Traits::LeafIndexSet,m>
  {
  public:
    LeafAssembledP1FEOperator (const G& grid)
      : AssembledP1FEOperator<G,RT,typename G::Traits::LeafIndexSet,m>(grid,grid.leafIndexSet())
    {}
  };


  template<class G, class RT, int m=1>
  class LevelAssembledP1FEOperator : public AssembledP1FEOperator<G,RT,typename G::Traits::LevelIndexSet,m>
  {
  public:
    LevelAssembledP1FEOperator (const G& grid, int level)
      : AssembledP1FEOperator<G,RT,typename G::Traits::LevelIndexSet,m>(grid,grid.levelIndexSet(level))
    {}
  };


  /** @} */

}
#endif
