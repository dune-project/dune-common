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
#include <dune/istl/io.hh>
#define HAVE_SUPERLU
#include <dune/fem/feop/spmatrix.hh>

#include "stokesparameters.hh"
#include "dune/disc/shapefunctions/dgspace/monomialshapefunctions.hh"
#include <dune/disc/functions/dgfunction.hh>
#include "boundaryconditions.hh"
#include "testfunctions.hh"
#include "rhs.hh"

namespace Dune
{
  template<class G,int ordr>
  class DGFiniteElementMethod
  {
    //dimension of grid
    enum {dim=G::dimension};
    enum { dimw=G::dimensionworld };
  public:
    //Grid
    typedef G Grid;
    //coordinate type
    typedef typename Grid::ctype ctype;
    typedef Dune::FieldVector< double , dim> Gradient;
    typedef Dune::FieldMatrix< double , dim, dim> InverseJacobianMatrix;
    // "order" is order of velocity shapefn
    // order of pressure shapefn  is (order-1) i.e, assumed one order less than that of velocity
    enum {order = ordr};
    //local vector and matrix blocks
    // local block size is sum of velocity dof and pressure dof
    //block size = dim*vdof.size() + pdof.size()
    static const int BlockSize =dim*Dune::MonomialShapeFunctionSetSize<dim,ordr>::maxSize+Dune::MonomialShapeFunctionSetSize<dim,ordr-1>::maxSize;
    typedef Dune::FieldVector<double,BlockSize> LocalVectorBlock;
    typedef Dune::FieldMatrix<double,BlockSize,BlockSize> LocalMatrixBlock;
    //shapefn
    typedef Dune::MonomialShapeFunctionSet<ctype,double,dim> ShapeFunctionSet;
    inline const ShapeFunctionSet & getVelocityShapeFunctionSet(Dune::GeometryType gt) const;
    inline const ShapeFunctionSet & getPressureShapeFunctionSet(Dune::GeometryType gt) const;

    typedef typename Grid::template Codim<0>::LevelIterator ElementIterator;
    typedef typename Grid::template Codim<0>::EntityPointer EntityPointer;
    typedef typename Grid::template Codim<0>::Entity Entity;
    typedef typename Grid::template Codim<dim>::LevelIterator VertexIterator;
    typedef typename Grid::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename Grid::template Codim<1>::EntityPointer InterSectionPointer;

    DGFiniteElementMethod (const DGStokesParameters& par, DirichletBoundary<G>& db, RightHandSide<G>& rh) : parameter(par),dirichletvalue(db), rhsvalue(rh) {};
    //local assembly
    void assembleVolumeTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const;
    void assembleFaceTerm(Entity& ep,IntersectionIterator& isp, LocalMatrixBlock& Aee,
                          LocalMatrixBlock& Aef,LocalMatrixBlock& Afe, LocalVectorBlock& Be) const;
    void assembleBoundaryTerm(Entity& ep, IntersectionIterator& isp, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const ;
    // stokes system has dim+1 variables (dim velocity comps and 1 pressure)
    double evaluateSolution(int variable,const Entity& element,const Dune::FieldVector<ctype,dim>& local,
                            const LocalVectorBlock& xe) const;
    double evaluateL2error(int variable,const ExactSolution<ctype, dim> & exact,const Entity& element,
                           const LocalVectorBlock& xe) const;

  private:

    Dune::MonomialShapeFunctionSetContainer<ctype,double,dim,order> space;
    DGStokesParameters parameter;
    DirichletBoundary<G> dirichletvalue;
    RightHandSide<G> rhsvalue;

  };



  template<class G,int ordr>
  class DGStokes
  {

  public:
    //dimension of grid
    enum {dimension=G::dimension};
    enum { dimensionworld=G::dimensionworld };
    typedef G Grid;
  private:
    enum {dim=G::dimension};
    enum { dimw=G::dimensionworld };
    typedef typename Grid::ctype ctype;
    // Iterators
    typedef typename Grid::template Codim<0>::LevelIterator ElementIterator;
    typedef typename Grid::template Codim<0>::EntityPointer EntityPointer;
    typedef typename Grid::template Codim<0>::Entity Entity;
    typedef typename Grid::template Codim<dim>::LevelIterator VertexIterator;
    typedef typename Grid::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename Grid::template Codim<1>::EntityPointer InterSectionPointer;
    static const int BlockSize =((dim*Dune::MonomialShapeFunctionSetSize<dim,ordr>::maxSize)+(Dune::MonomialShapeFunctionSetSize<dim,ordr-1>::maxSize));
    typedef typename DGFiniteElementMethod<G,ordr>::Gradient Gradient;
    typedef typename DGFiniteElementMethod<G,ordr>::LocalVectorBlock LocalVectorBlock;
    typedef typename DGFiniteElementMethod<G,ordr>::LocalMatrixBlock LocalMatrixBlock;
    typedef Dune::BlockVector<LocalVectorBlock> Vector;
    typedef Dune::BCRSMatrix<LocalMatrixBlock> Matrix;
    typedef Dune::LevelDGFunction<Grid, double, ordr> DGFunction;
  public:
    //inline constructor with initializer list
    //DGStokes(Grid &g) : grid(g), dgfem(),x(grid, level), exact(){};
    DGStokes(Grid &g, ExactSolution<ctype,dim>& ex, DGStokesParameters& par, DirichletBoundary<Grid>& db, RightHandSide<Grid>& rh ) : grid(g),exact(ex), dgfem(par,db, rh),x(grid, level) {};

    // global assembly and solving
    void assembleStokesSystem() ;
    void solveStokesSystem();
    double evaluateSolution(const EntityPointer & e,
                            const Dune::FieldVector<ctype, dim> & local) const;

    //l2error computation
    // stokes system has dim+1 variables (dim velocity comps and 1 pressure)
    double l2errorStokesSystem(int variable) const;

    const DGFunction & solution() const { return x; }

  private:
    typedef typename DGFiniteElementMethod<G,ordr>::ShapeFunctionSet ShapeFunctionSet;
    inline const ShapeFunctionSet & getVelocityShapeFunctionSet(const EntityPointer &ep) const;
    inline const ShapeFunctionSet & getPressureShapeFunctionSet(const EntityPointer &ep) const;
  public:
    Grid & grid;
    int level;
    ExactSolution<ctype,dim>& exact;
  private:
    DGFiniteElementMethod<G,ordr> dgfem;
    Dune::SparseRowMatrix<double> AA;
    Dune::SimpleVector<double> bb;
    Matrix A;
    Vector b;
    DGFunction x;

  };



#include "dgstokes.cc"

} // end of namespace

#endif
