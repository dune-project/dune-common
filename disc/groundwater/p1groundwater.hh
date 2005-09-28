// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_P1GROUNDWATER_HH__
#define __DUNE_P1GROUNDWATER_HH__

#include <iostream>
#include <iomanip>

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"
#include "istl/operators.hh"
#include "istl/bvector.hh"
#include "istl/bcrsmatrix.hh"

#include "disc/shapefunctions/lagrangeshapefunctions.hh"
#include "disc/operators/p1operator.hh"
#include "disc/functions/p1function.hh"
#include "groundwater.hh"

/**
 * @file
 * @brief  compute local stiffness matrix for conforming finite elements for diffusion equation
 * @author Peter Bastian
 */


namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief compute local stiffness matrix for conforming finite elements for diffusion equation
   *
   */


  //! A class for computing local stiffness matrices
  /*! A class for computing local stiffness matrice for the
        diffusion equation

            div j = q; j = -K grad u; in Omega

                u = g on Gamma1; j*n = J on Gamma2.

        Uses conforming finite elements with the Lagrange shape functions.
        It should work for dimensions and element types.
        All the numbering is with respect to the reference element and the
        Lagrange shape functions

        Template parameters are:

        - Grid  a DUNE grid type
        - RT    type used for return values
   */
  template<class G, class RT>
  class LagrangeFEMForGroundwaterEquation {
    typedef typename G::ctype DT;
    enum {n=G::dimension, m=1};
    typedef typename G::Traits::template Codim<0>::Entity Entity;
    typedef typename G::Traits::IntersectionIterator IntersectionIterator;
    enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize};

  public:
    //! Constructor
    LagrangeFEMForGroundwaterEquation (const GroundwaterEquationParameters<G,RT>& params, bool procBoundaryAsDirichlet_=true)
      : problem(params)
    {
      for (int i=0; i<n; i++)
      {
        b[i] = 0;
        bctype[i] = GroundwaterEquationParameters<G,RT>::neumann;
        for (int j=0; j<n; j++) A[i][j] = 0;
      }
      currentsize = 0;
      procBoundaryAsDirichlet = procBoundaryAsDirichlet_;
    }


    //! assemble local stiffness matrix for given element and order
    /*! On exit the following things have been done:
       - The stiffness matrix for the given entity and polynomial degree has been assembled and is
       accessible with the mat() method.
       - The boundary conditions have been evaluated and are accessible with the bc() method
       - The right hand side has been assembled. It contains either the value of the essential boundary
       condition or the assembled source term and neumann boundary condition. It is accessible via the rhs() method.
       @param[in]  e    a codim 0 entity reference
       @param[in]  k    order of Lagrange basis
     */
    void assemble (const Entity& e, int k)
    {
      // extract some important parameters
      Dune::GeometryType gt = e.geometry().type();
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,k);
      DT Zero = 0;
      double refvolume = Dune::ReferenceElements<DT,n>::general(gt).volume();
      currentsize = sfs.size();

      // clear assemble data
      for (int i=0; i<sfs.size(); i++)
      {
        b[i] = 0;
        bctype[i] = GroundwaterEquationParameters<G,RT>::neumann;
        for (int j=0; j<sfs.size(); j++)
          A[i][j] = 0;
      }

      // Loop over all quadrature points and assemble matrix and right hand side
      int p=2*k;
      for (int g=0; g<Dune::QuadratureRules<DT,n>::rule(gt,p).size(); ++g)     // run through all quadrature points
      {
        const Dune::FieldVector<DT,n>& local = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].position();           // pos of integration point
        Dune::FieldVector<DT,n> global = e.geometry().global(local);                                            // ip in global coordinates
        const Dune::FieldMatrix<DT,n,n>& jac = e.geometry().jacobianInverse(local);                             // eval jacobian inverse
        const Dune::FieldMatrix<DT,n,n>& K = problem.K(global,e,local);                                         // eval diffusion tensor
        double weight = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].weight();                                    // weight of quadrature point
        DT detjac = e.geometry().integrationElement(local);                                                     // determinant of jacobian
        RT q = problem.q(global,e,local);           // source term
        RT factor = weight*refvolume*detjac;

        // evaluate gradients at Gauss points
        Dune::FieldVector<DT,n> grad[SIZE], temp, gv;
        for (int i=0; i<sfs.size(); i++)
        {
          for (int l=0; l<n; l++)
            temp[l] = sfs[i].evaluateDerivative(0,l,local);
          grad[i] = 0;
          jac.umv(temp,grad[i]);                 // transform gradient to global ooordinates
        }

        for (int i=0; i<sfs.size(); i++)           // loop over test function number
        {
          // rhs
          b[i] += q*sfs[i].evaluateFunction(0,local)*factor;

          // matrix
          gv = 0; K.umv(grad[i],gv);               // multiply with diffusion tensor
          A[i][i] += (grad[i]*gv)*factor;
          for (int j=0; j<i; j++)
          {
            RT t = (grad[j]*gv)*factor;
            A[i][j] += t;
            A[j][i] += t;
          }
        }
      }

      // evaluate boundary conditions via intersection iterator
      IntersectionIterator endit = e.iend();
      for (IntersectionIterator it = e.ibegin(); it!=endit; ++it)
      {
        // if we have a neighbor then we assume there is no boundary
        // (it might still be an interior boundary ... )
        if (it.neighbor()) continue;

        // determine boundary condition type for this face, initialize with processor boundary
        typename GroundwaterEquationParameters<G,RT>::BC bctypeface = GroundwaterEquationParameters<G,RT>::process;

        // handle face on exterior boundary
        if (it.boundary())
        {
          Dune::GeometryType gtface = it.intersectionSelfLocal().type();
          double refvolumeface = Dune::ReferenceElements<DT,n-1>::general(gtface).volume();
          for (int g=0; g<Dune::QuadratureRules<DT,n-1>::rule(gtface,p).size(); ++g)
          {
            const Dune::FieldVector<DT,n-1>& facelocal = Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].position();
            FieldVector<DT,n> local = it.intersectionSelfLocal().global(facelocal);
            FieldVector<DT,n> global = it.intersectionGlobal().global(Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].position());
            bctypeface = problem.bctype(global,e,local);                       // eval bctype
            if (bctypeface!=GroundwaterEquationParameters<G,RT>::neumann) break;
            RT J = problem.J(global,e,local);
            double weightface = Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].weight();
            DT detjacface = it.intersectionGlobal().integrationElement(facelocal);
            for (int i=0; i<sfs.size(); i++)                       // loop over test function number
              if (bctype[i]==GroundwaterEquationParameters<G,RT>::neumann)
                b[i] -= J*sfs[i].evaluateFunction(0,local)*weightface*refvolumeface*detjacface;
          }
          if (bctypeface==GroundwaterEquationParameters<G,RT>::neumann) continue;                 // was a neumann face, go to next face
        }

        // If we are here, then its either an exterior boundary face with Dirichlet condition
        // or a processor boundary (i.e. neither boundary() nor neighbor() was true)
        // How processor boundaries are handled depends on the processor boundary mode
        if (bctypeface==GroundwaterEquationParameters<G,RT>::process && procBoundaryAsDirichlet==false)
          continue;               // then it acts like homogeneous Neumann

        // now handle exterior or interior Dirichlet boundary
        for (int i=0; i<sfs.size(); i++)           // loop over test function number
        {
          if (sfs[i].codim()==0) continue;                 // skip interior dof
          if (sfs[i].codim()==1)                 // handle face dofs
          {
            if (sfs[i].entity()==it.numberInSelf())
            {
              if (bctype[i]<bctypeface)
              {
                bctype[i] = bctypeface;
                if (bctypeface==GroundwaterEquationParameters<G,RT>::process)
                  b[i] = 0;
                if (bctypeface==GroundwaterEquationParameters<G,RT>::dirichlet)
                {
                  Dune::FieldVector<DT,n> global = e.geometry().global(sfs[i].position());
                  b[i] = problem.g(global,e,sfs[i].position());
                }
              }
            }
            continue;
          }
          // handle subentities of this face
          for (int j=0; j<ReferenceElements<DT,n>::general(gt).size(it.numberInSelf(),1,sfs[i].codim()); j++)
            if (sfs[i].entity()==ReferenceElements<DT,n>::general(gt).subentity(it.numberInSelf(),1,j,sfs[i].codim()))
            {
              if (bctype[i]<bctypeface)
              {
                bctype[i] = bctypeface;
                if (bctypeface==GroundwaterEquationParameters<G,RT>::process)
                  b[i] = 0;
                if (bctypeface==GroundwaterEquationParameters<G,RT>::dirichlet)
                {
                  Dune::FieldVector<DT,n> global = e.geometry().global(sfs[i].position());
                  b[i] = problem.g(global,e,sfs[i].position());
                }
              }
            }
        }
      }
    }

    // print contents of local stiffness matrix
    void print (std::ostream& s, int width, int precision)
    {
      // set the output format
      s.setf(std::ios_base::scientific, std::ios_base::floatfield);
      int oldprec = s.precision();
      s.precision(precision);

      for (int i=0; i<currentsize; i++)
      {
        s << "FEM";              // start a new row
        s << " ";                // space in front of each entry
        s.width(4);              // set width for counter
        s << i;                  // number of first entry in a line
        for (int j=0; j<currentsize; j++)
        {
          s << " ";                         // space in front of each entry
          s.width(width);                   // set width for each entry anew
          s << A[i][j];                     // yeah, the number !
        }
        s << " ";                   // space in front of each entry
        s.width(width);             // set width for each entry anew
        s << b[i];
        s << " ";                   // space in front of each entry
        s.width(width);             // set width for each entry anew
        s << bctype[i];
        s << std::endl;          // start a new line
      }


      // reset the output format
      s.precision(oldprec);
      s.setf(std::ios_base::fixed, std::ios_base::floatfield);
    }

    //! access local stiffness matrix
    /*! Access elements of the local stiffness matrix. Elements are
       undefined without prior call to the assemble method.
     */
    RT mat (int i, int j) const
    {
      return A[i][j];
    }

    //! access right hand side
    /*! Access elements of the right hand side vector. Elements are
       undefined without prior call to the assemble method.
     */
    RT rhs (int i) const
    {
      return b[i];
    }

    //! access boundary condition for each dof
    /*! Access boundary condition type for each degree of freedom. Elements are
       undefined without prior call to the assemble method.
     */
    typename GroundwaterEquationParameters<G,RT>::BC bc (int i) const
    {
      return bctype[i];
    }

  private:
    int currentsize;
    const GroundwaterEquationParameters<G,RT>& problem;
    RT A[SIZE][SIZE];
    RT b[SIZE];
    typename GroundwaterEquationParameters<G,RT>::BC bctype[SIZE];
    bool procBoundaryAsDirichlet;
  };


  //! The global stiffness matrix
  template<class G, class RT, class IS>
  class P1GroundwaterOperator : public AssembledP1FEOperator<G,RT,IS>
  {
    typedef typename G::ctype DT;
    enum {n=G::dimension};
    typedef typename G::Traits::template Codim<0>::Entity Entity;
    typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
    typedef typename P1FEFunction<G,RT,IS>::RepresentationType VectorType;
    typedef typename AssembledP1FEOperator<G,RT,IS>::RepresentationType MatrixType;
    typedef typename MatrixType::RowIterator rowiterator;
    typedef typename MatrixType::ColIterator coliterator;

  public:
    P1GroundwaterOperator (const G& g, const GroundwaterEquationParameters<G,RT>& params,
                           const IS& indexset, bool procBoundaryAsDirichlet=true)
      : AssembledP1FEOperator<G,RT,IS>(g,indexset), loc(params,procBoundaryAsDirichlet)
    {       }

    //! assemble operator, rhs and Dirichlet boundary conditions
    template<class FEFunc>
    void assemble (FEFunc& u, FEFunc& f)
    {
      // clear global stiffness matrix and right hand side
      this->A = 0;
      *f = 0;

      // allocate flag vector to hold flags for essential boundary conditions
      std::vector<unsigned char> essential(this->vertexmapper.size());
      for (int i=0; i<essential.size(); i++) essential[i] = GroundwaterEquationParameters<G,RT>::neumann;

      // run over all leaf elements
      Iterator eendit = this->is.template end<0,All_Partition>();
      for (Iterator it = this->is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        // get access to shape functions for P1 elements
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type&
        sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);

        // build local stiffness matrix for P1 elements
        // inludes rhs and boundary condition information
        loc.assemble(*it,1);           // assemble local stiffness matrix

        // accumulate local matrix into global matrix
        for (int i=0; i<sfs.size(); i++)           // loop over rows
        {
          if (sfs[i].codim()!=n) DUNE_THROW(MathError,"expected codim=dim");
          int alpha = this->vertexmapper.template submap<n>(*it,sfs[i].entity());

          // accumulate matrix
          for (int j=0; j<sfs.size(); j++)
          {
            if (sfs[j].codim()!=n) DUNE_THROW(MathError,"expected codim=dim");
            int beta = this->vertexmapper.template submap<n>(*it,sfs[j].entity());
            this->A[alpha][beta] += loc.mat(i,j);
          }

          // accumulate essential boundary condition
          if (loc.bc(i)>essential[alpha])
          {
            essential[alpha] = loc.bc(i);
            (*f)[alpha] = loc.rhs(i);
          }

          // accumulate rhs
          if (essential[alpha]==GroundwaterEquationParameters<G,RT>::neumann)
            (*f)[alpha] += loc.rhs(i);
        }
      }


      // put in essential boundary conditions
      rowiterator endi=this->A.end();
      for (rowiterator i=this->A.begin(); i!=endi; ++i)
      {
        if (essential[i.index()]!=GroundwaterEquationParameters<G,RT>::neumann)
        {
          coliterator endj=(*i).end();
          for (coliterator j=(*i).begin(); j!=endj; ++j)
            if (j.index()==i.index())
              (*j) = 1;
            else
              (*j) = 0;
          (*u)[i.index()] = (*f)[i.index()];
        }
      }

      // print it
      //	  printmatrix(std::cout,this->A,"global stiffness matrix","row",9,1);
    }

  private:
    LagrangeFEMForGroundwaterEquation<G,RT> loc;
  };


  template<class G, class RT>
  class LeafP1GroundwaterOperator : public P1GroundwaterOperator<G,RT,typename G::Traits::LeafIndexSet>
  {
  public:
    LeafP1GroundwaterOperator (const G& grid, const GroundwaterEquationParameters<G,RT>& params,
                               bool procBoundaryAsDirichlet=true)
      : P1GroundwaterOperator<G,RT,typename G::Traits::LeafIndexSet>(grid,params,grid.leafIndexSet(),procBoundaryAsDirichlet)
    {}
  };


  template<class G, class RT>
  class LevelP1GroundwaterOperator : public P1GroundwaterOperator<G,RT,typename G::Traits::LevelIndexSet>
  {
  public:
    LevelP1GroundwaterOperator (const G& grid, int level,
                                const GroundwaterEquationParameters<G,RT>& params, bool procBoundaryAsDirichlet=true)
      : P1GroundwaterOperator<G,RT,typename G::Traits::LevelIndexSet>(grid,params,grid.levelIndexSet(level),procBoundaryAsDirichlet)
    {}
  };


  /** @} */
}
#endif
