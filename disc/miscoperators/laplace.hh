// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P1_LAPLACE_ASSEMBLER_HH
#define DUNE_P1_LAPLACE_ASSEMBLER_HH

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"

#include <dune/quadrature/quadraturerules.hh>

#include "disc/shapefunctions/lagrangeshapefunctions.hh"
#include <dune/disc/operators/localstiffness.hh>

/**
 * @file
 * @brief  Compute local stiffness matrix for the Laplace operator
 * @author Oliver Sander
 */


namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief Compute local stiffness matrix for the Laplace operator
   *
        Uses conforming finite elements with the Lagrange shape functions.
        It should work for all dimensions and element types.
        All the numbering is with respect to the reference element and the
        Lagrange shape functions

        Template parameters are:

        - Grid  a DUNE grid type
        - RT    type used for return values
   */
  template<class GridType, class RT>
  class LaplaceLocalStiffness : public LocalStiffness<GridType, RT, 1>
  {
    // grid types
    typedef typename GridType::ctype DT;
    typedef typename GridType::Traits::template Codim<0>::Entity Entity;
    typedef typename GridType::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename GridType::Traits::IntersectionIterator IntersectionIterator;

    // some other sizes
    enum {dim=GridType::dimension};

  public:

    /** \brief Number of components for the global assembler to collect */
    enum {m=1};

    // types for matrics, vectors and boundary conditions

    /** \brief One entry in the stiffness matrix */
    typedef FieldMatrix<RT,m,m> MBlockType;

    /** \brief One entry in the global vectors */
    typedef FieldVector<RT,m> VBlockType;

    /** \brief Componentwise boundary conditions */
    typedef FixedArray<BoundaryConditions::Flags,m> BCBlockType;

    //! Constructor
    LaplaceLocalStiffness (bool procBoundaryAsDirichlet_=true) {

      procBoundaryAsDirichlet = procBoundaryAsDirichlet_;

      // For the time being:  all boundary conditions are homogeneous Neumann
      // This means no boundary condition handling is done at all
      for (int i=0; i<LocalStiffness<GridType,RT,m>::SIZE; i++)
        this->bctype[i] = BoundaryConditions::neumann;
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
      GeometryType gt = e.geometry().type();
      const typename LagrangeShapeFunctionSetContainer<DT,RT,dim>::value_type& sfs
        = LagrangeShapeFunctions<DT,RT,dim>::general(gt,k);

      // clear assemble data
      for (int i=0; i<sfs.size(); i++) {
        this->b[i] = 0;
        this->bctype[i][0] = BoundaryConditions::neumann;
        for (int j=0; j<sfs.size(); j++)
          this->A[i][j] = 0;
      }

      // Loop over all quadrature points and assemble matrix and right hand side

      /** \todo Compute suitable quadrature order */
      int p=dim;
      if (gt.isPrism() || gt.isPyramid())
        p = 2;

      p *= k;

      for (size_t g=0; g<Dune::QuadratureRules<DT,dim>::rule(gt,p).size(); ++g) {

        // pos of integration point
        const Dune::FieldVector<DT,dim>& quadPos = Dune::QuadratureRules<DT,dim>::rule(gt,p)[g].position();

        // weight of quadrature point
        double weight = Dune::QuadratureRules<DT,dim>::rule(gt,p)[g].weight();

        const FieldMatrix<double,dim,dim>& inv = e.geometry().jacobianInverseTransposed(quadPos);

        // determinant of jacobian
        DT detjac = e.geometry().integrationElement(quadPos);

        RT factor = weight*detjac;

        // evaluate gradients at Gauss points
        Dune::FieldVector<DT,dim> grad[LocalStiffness<GridType,RT,m>::SIZE], temp;
        for (int i=0; i<sfs.size(); i++)
        {
          for (int l=0; l<dim; l++)
            temp[l] = sfs[i].evaluateDerivative(0,l,quadPos);
          grad[i] = 0;
          inv.umv(temp,grad[i]);             // transform gradient to global ooordinates
        }

        // loop over test function number
        for (int i=0; i<sfs.size(); i++) {

          // matrix
          this->A[i][i] += (grad[i]*grad[i])*factor;
          for (int j=0; j<i; j++)
          {
            RT t = (grad[j]*grad[i])*factor;
            this->A[i][j] += t;
            this->A[j][i] += t;
          }
        }
      }

#if 0
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
            FieldVector<DT,dim> local = it.intersectionSelfLocal().global(facelocal);
            FieldVector<DT,dim> global = it.intersectionGlobal().global(facelocal);
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
                  Dune::FieldVector<DT,dim> global = e.geometry().global(sfs[i].position());
                  b[i] = problem.g(global,e,sfs[i].position());
                }
              }
            }
            continue;
          }
          // handle subentities of this face
          for (int j=0; j<ReferenceElements<DT,dim>::general(gt).size(it.numberInSelf(),1,sfs[i].codim()); j++)
            if (sfs[i].entity()==ReferenceElements<DT,dim>::general(gt).subEntity(it.numberInSelf(),1,j,sfs[i].codim()))
            {
              if (bctype[i][0]<bctypeface)
              {
                bctype[i][0] = bctypeface;
                if (bctypeface==BoundaryConditions::process)
                  b[i] = 0;
                if (bctypeface==BoundaryConditions::dirichlet)
                {
                  Dune::FieldVector<DT,dim> global = e.geometry().global(sfs[i].position());
                  b[i] = problem.g(global,e,sfs[i].position());
                }
              }
            }
        }
      }
#endif
    }

    //! should allow to assmble boundary conditions only
    void assembleBoundaryCondition (const Entity& e, int k=1)
    {}

  private:
    // parameters given in constructor
    bool procBoundaryAsDirichlet;

  };

  /** @} */
}
#endif
