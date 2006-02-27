// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DGSTOKES_HH
#define DUNE_DGSTOKES_HH

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"
#include "istl/operators.hh"
#include "istl/bvector.hh"
#include "istl/bcrsmatrix.hh"

#define HAVE_SUPERLU
#include <dune/fem/feop/spmatrix.hh>

#include "stokesequation.hh"
#include "dune/disc/shapefunctions/dgspace/monomialshapefunctions.hh"

namespace Dune
{



  template<class G,int ordr>
  class DGStokes
  {
    //dimension of grid
    enum {dim=G::dimension};
    enum { dimw=G::dimensionworld };
  public:
    typedef G Grid;
    //domain field type
    typedef typename Grid::ctype ctype;

    typedef Dune::FieldVector< double , dim> Gradient;
    typedef Dune::FieldMatrix< double , dim, dim> InverseJacobianMatrix;


    //shapefn order
    // "order" is order of velocity shapefn
    // order of pressure shapefn  is (order-1) i.e, always one order less than that of velocity
    enum {order = ordr};
    //shapefn
    typedef Dune::MonomialShapeFunctionSet<ctype,double,dim> ShapeFunctionSet;
    inline const ShapeFunctionSet & getShapeFunctionSet(Dune::GeometryType gt) const;
  private:
    // Iterators
    typedef typename Grid::template Codim<0>::LevelIterator ElementIterator;
    typedef typename Grid::template Codim<0>::EntityPointer EntityPointer;
    typedef typename Grid::template Codim<0>::Entity Entity;
    typedef typename Grid::template Codim<dim>::LevelIterator VertexIterator;
    typedef typename Grid::template Codim<0>::IntersectionIterator IntersectionIterator;
    //local vector and matrix blocks
    // use of spmatrix inorder as the solver is superLU
    // local block size is sum of velocity dof and pressure dof
    //block size = 2*vdof.size() + pdof.size()
    static const int BlockSize =3*Dune::MonomialShapeFunctionSetSize<dim,ordr>::maxSize;
    typedef Dune::SparseRowMatrix<double> LocalMatrixBlock;
    typedef Dune::SimpleVector<double> LocalVectorBlock;

  public:
    DGStokes(Grid &g) : grid(g) {};
    // assembling local matrix entries
    void assembleVolumeTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const;
    void assembleFaceTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const;
    void assembleBoundaryTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const ;
    // global assembly and solving
    void assembleStokesSystem() ;
    void solveStokesSystem();
  public:
    Grid & grid;
    int level;

  };

#include "dgstokes.cc"

} // end of namespace

#endif
