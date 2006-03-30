// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1MGTRANSFER_HH
#define DUNE_P1MGTRANSFER_HH

#include <set>
#include <map>

#include "dune/common/exceptions.hh"
#include "dune/common/fvector.hh"
#include "dune/common/fmatrix.hh"
#include "dune/common/geometrytype.hh"
#include "dune/grid/common/grid.hh"
#include "dune/grid/common/mcmgmapper.hh"
#include "dune/disc/shapefunctions/lagrangeshapefunctions.hh"
#include "dune/istl/bvector.hh"
#include "dune/istl/bcrsmatrix.hh"

/**
 * @brief  Classes for multigrid transfer operators assembled in sparse matrices
 * @author Peter Bastian
 */


namespace Dune
{
  /** @addtogroup DISC_Operators
   *
   * @{
   */


  /*! @brief A class for mapping a P1 function to a P1 function

     This class sets up a compressed row storage matrix with connectivity for P1 elements.
     It includes hanging nodes and is able to extend the matrix pattern arising
     from non-overlapping grids to minimum overlap.

     This class does not fill any entries into the matrix.
   */
  template<class G, class RT, int m=1>
  class P1MGTransfer
  {
    // export type used to store the matrix
    typedef FieldMatrix<RT,m,m> BlockType;
    typedef BCRSMatrix<BlockType> RepresentationType;
    typedef typename RepresentationType::RowIterator rowiterator;
    typedef typename RepresentationType::ColIterator coliterator;

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

    typedef typename G::ctype DT;
    enum {n=G::dimension};
    typedef typename G::template Codim<0>::Entity Entity;
    typedef typename G::template Codim<0>::LevelIterator ElementLevelIterator;
    typedef typename G::template Codim<n>::LevelIterator VertexLevelIterator;
    typedef typename G::template Codim<0>::EntityPointer EntityPointer;
    typedef typename G::template Codim<0>::LevelIndexSet IS;
    typedef MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout> VM;

    typedef std::set<int> IntSet;
    typedef std::map<int,IntSet> Graph;

  public:

    /** \brief create a sparse matrix with structure for interpolation
            from coarse to fine grid
     */
    P1MGTransfer (const G& grid_, int level_)
      : grid(grid_), level(level_)
    {
      // check that coarse grid exists
      if (level==0)
        DUNE_THROW(Exception,"P1MGTransfer: level greater 0 required");

      // allocate vertex mappers for the fine and coarse grid
      VM finemapper(grid,grid.levelIndexSet(level));
      VM coarsemapper(grid,grid.levelIndexSet(level-1));

      // allocate a graph structure for the sparsity pattern (yes we are lazy here)
      Graph graph;

      // allocate a flag vector to handle each vertex exactly once
      std::vector<bool> treated(finemapper.size(),false);

      // build the graph structure from the mesh
      for (ElementLevelIterator it = grid.template lbegin<0>(level);
           it!=grid.template lend<0>(level); ++it)
      {
        // get geometry type of entity
        GeometryType gt = it->geometry().type();

        // get father entity
        const EntityPointer father = it->father();

        // get geometry type of father
        GeometryType gtf = father->geometry().type();

        // connect every vertex of the fine grid element with
        // with every vertex of the coarse grid element
        for (int i=0; i<it->template count<n>(); i++)
        {
          // get index of fine grid vertex
          int indexi = finemapper.template map<n>(*it,i);

          // skip vertices that have already been treated
          if (treated[indexi]) continue;

          // get position of fine grid vertex in local coordinate system of father
          const FieldVector<DT,n>& cpos=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position();
          FieldVector<DT,n> pos = it->geometryInFather().global(cpos);

          // determine interpolation weights
          for (int j=0; j<father-> template count<n>(); j++)
          {
            // get value of j th basis function in father element
            RT phi = Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1)[j].evaluateFunction(0,pos);

            // get global index of that basis function
            int indexj = coarsemapper.template map<n>(*father,j);

            // insert only if non-zero
            if (std::abs(phi)>1E-6) graph[indexi].insert(indexj);
          }
          treated[indexi] = true;
        }
      }

      // compute number of nonzeroes
      int nnz=0;
      for (Graph::iterator i=graph.begin(); i!=graph.end(); ++i)
        nnz += i->second.size();

      // allocate the matrix
      std::cout << grid.comm().rank() << ": making " << finemapper.size() << "x"
                << coarsemapper.size() << " transfer matrix with " << nnz
                << " nonzeroes" << std::endl;
      A = new RepresentationType(finemapper.size(),coarsemapper.size(),nnz,RepresentationType::random);

      // set row sizes
      for (Graph::iterator i=graph.begin(); i!=graph.end(); ++i)
        A->setrowsize(i->first,i->second.size());
      A->endrowsizes();

      // set the structure
      for (Graph::iterator i=graph.begin(); i!=graph.end(); ++i)
        for (IntSet::iterator j=i->second.begin(); j!=i->second.end(); ++j)
          A->addindex(i->first,*j);
      A->endindices();

      // deallocate the map
      graph.clear();
    }


    /** \brief assemble the entries of the interpolation matrix; do not interpolate
            to vertices with essential boundary conditions.
     */
    void assemble (LocalStiffness<G,RT,m>& loc)
    {
      // allocate vertex mappers for the fine and coarse grid
      VM finemapper(grid,grid.levelIndexSet(level));
      VM coarsemapper(grid,grid.levelIndexSet(level-1));

      // allocate a flag vector to handle each vertex exactly once
      std::vector<bool> treated(finemapper.size(),false);

      // assemble the entries; needs to consider Dirichlet boundary conditions later
      for (ElementLevelIterator it = grid.template lbegin<0>(level);
           it!=grid.template lend<0>(level); ++it)
      {
        // get geometry type of entity
        GeometryType gt = it->geometry().type();

        // get father entity
        const EntityPointer father = it->father();

        // get geometry type of father
        GeometryType gtf = father->geometry().type();

        // connect every vertex of the fine grid element with
        // with every vertex of the coarse grid element
        for (int i=0; i<it->template count<n>(); i++)
        {
          // get index of fine grid vertex
          int indexi = finemapper.template map<n>(*it,i);

          // skip vertices that have already been treated
          if (treated[indexi]) continue;

          // get position of fine grid vertex in local coordinate system of father
          const FieldVector<DT,n>& cpos=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position();
          FieldVector<DT,n> pos = it->geometryInFather().global(cpos);

          // compute and set matrix values
          for (int j=0; j<father->template count<n>(); ++j)
          {
            // get value of j th basis function in father element
            RT phi = Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1)[j].evaluateFunction(0,pos);

            // insert entry if large enough
            if (std::abs(phi)>1E-6)
            {
              // get global index of that basis function
              int indexj = coarsemapper.template map<n>(*father,j);

              // fill diagonal matrix block;
              BlockType D;
              for (int compi=0; compi<m; compi++)
                for (int compj=0; compj<m; compj++)
                  if (compi==compj)
                    D[compi][compj] = phi;
                  else
                    D[compi][compj] = 0.0;

              // set entry
              (*A)[indexi][indexj] = D;
            }
          }

          // mark this vertex as done
          treated[indexi] = true;
        }
      }


      // second round: eliminate interpolation in rows with essential boundary conditions
      for (ElementLevelIterator it = grid.template lbegin<0>(level);
           it!=grid.template lend<0>(level); ++it)
      {
        // assemble boundary conditions for the given element
        loc.assembleBoundaryCondition(*it);

        // look at all vertices to detect essential boundary conditions
        for (int i=0; i<it->template count<n>(); i++)
        {
          // get index of fine grid vertex
          int indexi = finemapper.template map<n>(*it,i);

          // go through all components at this vertex
          for (int compi=0; compi<m; compi++)
            if (loc.bc(i)[compi]==BoundaryConditions::process ||
                loc.bc(i)[compi]==BoundaryConditions::dirichlet)
            {
              // we have to set this row to zero
              coliterator colend = (*A)[indexi].end();
              for (coliterator colit=(*A)[indexi].begin(); colit!=colend; ++colit)
                for (int compj=0; compj<m; compj++)
                  (*colit)[compi][compj] = 0;
            }
        }
      }

      //          printmatrix(std::cout,*A,"prolongation matrix","row",9,1);

    }

    //! return const reference to operator matrix
    const RepresentationType& operator* () const
    {
      return *A;
    }

    //! return reference to operator matrix
    RepresentationType& operator* ()
    {
      return *A;
    }

    //! makes matrix consistent in nonoverlapping case

    // destructor
    ~P1MGTransfer ()
    {
      delete A;
    }

  private:

    // make copy constructor and assignment private
    P1MGTransfer (const P1MGTransfer&) {}
    P1MGTransfer& operator= (const P1MGTransfer&) {}

    // data members
    const G& grid;
    int level;
    RepresentationType* A;
  };



  /** @} */
}
#endif
