// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1GROUNDWATER_HH
#define DUNE_P1GROUNDWATER_HH

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>

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
#include "disc/operators/boundaryconditions.hh"
#include "disc/functions/p0function.hh"
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
  /*! A class for computing local stiffness matrix for the
        diffusion equation

            div j = q; j = -K grad u; in Omega

                u = g on Gamma1; j*n = J on Gamma2.

        Uses conforming finite elements with the Lagrange shape functions.
        It should work for all dimensions and element types.
        All the numbering is with respect to the reference element and the
        Lagrange shape functions

        Template parameters are:

        - Grid  a DUNE grid type
        - RT    type used for return values
   */
  template<class G, class RT>
  class GroundwaterEquationLocalStiffness
  {
    // grid types
    typedef typename G::ctype DT;
    typedef typename G::Traits::template Codim<0>::Entity Entity;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename G::Traits::IntersectionIterator IntersectionIterator;

    // some other sizes
    enum {n=G::dimension};
    enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize, SIZEF=SIZE};

  public:
    // define the number of components of your system, this is used outside
    // to allocate the correct size of (dense) blocks with a FieldMatrix
    enum {m=1};

    // types for matrics, vectors and boundary conditions
    typedef FieldMatrix<RT,m,m> MBlockType;     // one entry in the stiffness matrix
    typedef FieldVector<RT,m> VBlockType;       // one entry in the global vectors
    typedef FixedArray<BoundaryConditions::Flags,m> BCBlockType;         // componentwise boundary conditions

    //! Constructor
    GroundwaterEquationLocalStiffness (const GroundwaterEquationParameters<G,RT>& params, bool procBoundaryAsDirichlet_=true)
      : problem(params)
    {
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
    void assemble (const Entity& e, int k=1)
    {
      // extract some important parameters
      Dune::GeometryType gt = e.geometry().type();
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,k);
      currentsize = sfs.size();

      //          std::cout << "Entity:" << std::endl;
      //          for (int i=0; i<e.template count<n>(); i++)
      //                std::cout << "corner i=" << i << " pos=" << e.geometry()[i] << std::endl;

      // clear assemble data
      for (int i=0; i<sfs.size(); i++)
      {
        b[i] = 0;
        bctype[i][0] = BoundaryConditions::neumann;
        for (int j=0; j<sfs.size(); j++)
          A[i][j] = 0;
      }

      // Loop over all quadrature points and assemble matrix and right hand side
      int p=2;
      if (gt==simplex) p=1;
      if (k>1) p=2*(k-1);
      for (size_t g=0; g<Dune::QuadratureRules<DT,n>::rule(gt,p).size(); ++g)     // run through all quadrature points
      {
        const Dune::FieldVector<DT,n>& local = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].position();           // pos of integration point
        Dune::FieldVector<DT,n> global = e.geometry().global(local);                                            // ip in global coordinates
        const Dune::FieldMatrix<DT,n,n> jac = e.geometry().jacobianInverseTransposed(local);                             // eval jacobian inverse
        const Dune::FieldMatrix<DT,n,n> K = problem.K(global,e,local);                                         // eval diffusion tensor
        double weight = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].weight();                                    // weight of quadrature point
        DT detjac = e.geometry().integrationElement(local);                                                     // determinant of jacobian
        RT q = problem.q(global,e,local);           // source term
        RT factor = weight*detjac;

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
        typename BoundaryConditions::Flags bctypeface = BoundaryConditions::process;

        // handle face on exterior boundary
        if (it.boundary())
        {
          Dune::GeometryType gtface = it.intersectionSelfLocal().type();
          for (size_t g=0; g<Dune::QuadratureRules<DT,n-1>::rule(gtface,p).size(); ++g)
          {
            const Dune::FieldVector<DT,n-1>& facelocal = Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].position();
            FieldVector<DT,n> local = it.intersectionSelfLocal().global(facelocal);
            FieldVector<DT,n> global = it.intersectionGlobal().global(facelocal);
            bctypeface = problem.bctype(global,e,local);                       // eval bctype

            //                            std::cout << "=== Boundary found"
            //                                                  << " facenumber=" << it.numberInSelf()
            //                                                  << " quadpoint=" << g
            //                                                  << " facelocal=" << facelocal
            //                                                  << " local=" << local
            //                                                  << " global=" << global
            //                                                  << std::endl;

            if (bctypeface!=BoundaryConditions::neumann) break;

            RT J = problem.J(global,e,local);
            double weightface = Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].weight();
            DT detjacface = it.intersectionGlobal().integrationElement(facelocal);
            for (int i=0; i<sfs.size(); i++)                       // loop over test function number
              if (bctype[i][0]==BoundaryConditions::neumann)
              {
                b[i] -= J*sfs[i].evaluateFunction(0,local)*weightface*detjacface;
                //                                              std::cout << "Neumann BC found, accumulating"
                //                                                                << -J*sfs[i].evaluateFunction(0,local)*weightface*detjacface
                //                                                                << std::endl;
                //                                              std::cout << "J=" << J << " shapef=" << sfs[i].evaluateFunction(0,local)
                //                                                                << " weight=" << weightface
                //                                                                << " detjac=" << detjacface << std::endl;
              }
          }
          if (bctypeface==BoundaryConditions::neumann) continue;                 // was a neumann face, go to next face
        }

        // If we are here, then its either an exterior boundary face with Dirichlet condition
        // or a processor boundary (i.e. neither boundary() nor neighbor() was true)
        // How processor boundaries are handled depends on the processor boundary mode
        if (bctypeface==BoundaryConditions::process && procBoundaryAsDirichlet==false)
          continue;               // then it acts like homogeneous Neumann

        // now handle exterior or interior Dirichlet boundary
        for (int i=0; i<sfs.size(); i++)           // loop over test function number
        {
          if (sfs[i].codim()==0) continue;                 // skip interior dof
          if (sfs[i].codim()==1)                 // handle face dofs
          {
            if (sfs[i].entity()==it.numberInSelf())
            {
              if (bctype[i][0]<bctypeface)
              {
                bctype[i][0] = bctypeface;
                if (bctypeface==BoundaryConditions::process)
                  b[i] = 0;
                if (bctypeface==BoundaryConditions::dirichlet)
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
            if (sfs[i].entity()==ReferenceElements<DT,n>::general(gt).subEntity(it.numberInSelf(),1,j,sfs[i].codim()))
            {
              if (bctype[i][0]<bctypeface)
              {
                bctype[i][0] = bctypeface;
                if (bctypeface==BoundaryConditions::process)
                  b[i] = 0;
                if (bctypeface==BoundaryConditions::dirichlet)
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
        s << bctype[i][0];
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
    const MBlockType& mat (int i, int j)
    {
      return A[i][j];
    }

    //! access right hand side
    /*! Access elements of the right hand side vector. Elements are
       undefined without prior call to the assemble method.
     */
    const VBlockType& rhs (int i)
    {
      return b[i];
    }

    //! access boundary condition for each dof
    /*! Access boundary condition type for each degree of freedom. Elements are
       undefined without prior call to the assemble method.
     */
    const BCBlockType bc (int i) const
    {
      return bctype[i];
    }

  private:
    // parameters given in constructor
    const GroundwaterEquationParameters<G,RT>& problem;
    bool procBoundaryAsDirichlet;

    // assembled data
    int currentsize;
    MBlockType A[SIZE][SIZE];
    VBlockType b[SIZE];
    BCBlockType bctype[SIZE];
  };

  /** @} */
}
#endif
