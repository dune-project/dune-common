// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P1_LINEAR_ELASTICITY_ASSEMBLER_HH
#define DUNE_P1_LINEAR_ELASTICITY_ASSEMBLER_HH

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
#include <dune/disc/operators/localstiffness.hh>

#include "disc/shapefunctions/lagrangeshapefunctions.hh"
#include "disc/operators/boundaryconditions.hh"

/**
 * @file
 * @brief  compute local stiffness matrix for conforming finite elements for linear elasticity equation
 * @author Oliver Sander
 */


namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief compute local stiffness matrix for the linear elasticity operator
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

        - GridType  a DUNE grid type
        - RT    type used for return values
   */
  template<class GridType, class RT>
  class LinearElasticityLocalStiffness : public LocalStiffness<GridType,RT,GridType::dimension>
  {
    // grid types
    typedef typename GridType::ctype DT;
    typedef typename GridType::Traits::template Codim<0>::Entity Entity;
    typedef typename GridType::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename GridType::Traits::IntersectionIterator IntersectionIterator;

    // some other sizes
    enum {dim=GridType::dimension};
    //enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,dim>::maxsize};

    //! The engineers' way of writing a symmetric second-order tensor
    typedef FieldVector<double, (dim+1)*dim/2> SymmTensor;

  public:
    // define the number of components of your system, this is used outside
    // to allocate the correct size of (dense) blocks with a FieldMatrix
    enum {m=dim};

    // types for matrics, vectors and boundary conditions
    typedef FieldMatrix<RT,m,m> MBlockType;     // one entry in the stiffness matrix
    typedef FieldVector<RT,m> VBlockType;       // one entry in the global vectors
    typedef FixedArray<BoundaryConditions::Flags,m> BCBlockType;         // componentwise boundary conditions
    // /////////////////////////////////
    //   The material parameters
    // /////////////////////////////////

    // Young's modulus
    double E_;

    // Poisson ratio
    double nu_;

    //! Constructor
    LinearElasticityLocalStiffness (bool procBoundaryAsDirichlet_=true)
      : E_(2.5e5)
    {

      //E_ = 2.5e5;
      nu_ = 0.3;

      this->currentsize_ = 0;
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

      this->currentsize_ = sfs.size();

      //          std::cout << "Entity:" << std::endl;
      //          for (int i=0; i<e.template count<n>(); i++)
      //                std::cout << "corner i=" << i << " pos=" << e.geometry()[i] << std::endl;

      // clear assemble data
      for (int i=0; i<sfs.size(); i++)
      {
        this->b[i] = 0;
        this->bctype[i][0] = BoundaryConditions::neumann;
        for (int j=0; j<sfs.size(); j++)
          this->A[i][j] = 0;
      }

      // Loop over all quadrature points and assemble matrix and right hand side

      // Compute suitable quadrature order
      int p=2;
      if (gt.isSimplex())
        p=1;
      if (k>1)
        p=2*(k-1);

      for (size_t g=0; g<Dune::QuadratureRules<DT,dim>::rule(gt,p).size(); ++g) {

        // pos of integration point
        const Dune::FieldVector<DT,dim>& local = Dune::QuadratureRules<DT,dim>::rule(gt,p)[g].position();

        // ip in global coordinates
        Dune::FieldVector<DT,dim> global = e.geometry().global(local);

        // eval jacobian inverse
        const Dune::FieldMatrix<DT,dim,dim> jac = e.geometry().jacobianInverseTransposed(local);

#if 0
        // eval diffusion tensor
        const Dune::FieldMatrix<DT,dim,dim> K = problem.K(global,e,local);
#endif

        // weight of quadrature point
        double weight = Dune::QuadratureRules<DT,dim>::rule(gt,p)[g].weight();

        // determinant of jacobian
        DT detjac = e.geometry().integrationElement(local);

#if 0
        // Source term;
        RT q = problem.q(global,e,local);
#endif

        RT factor = weight*detjac;

        // evaluate gradients at Gauss points
        Dune::FieldVector<DT,dim> grad[LocalStiffness<GridType,RT,m>::SIZE], temp, gv;
        for (int i=0; i<sfs.size(); i++) {

          for (int l=0; l<dim; l++)
            temp[l] = sfs[i].evaluateDerivative(0,l,local);
          grad[i] = 0;
          jac.umv(temp,grad[i]);         // transform gradient to global ooordinates

        }

        // /////////////////////////////////////////////
        //   Compute strain for all shape functions
        // /////////////////////////////////////////////
        std::vector<SymmTensor> strain(sfs.size()*dim);

        for (int i=0; i<sfs.size(); i++)
          for (int k=0; k<dim; k++) {

            // The deformation gradient of the shape function
            FieldMatrix<double, dim, dim> Grad(0);
            Grad[k] = grad[i];

            //std::cout << Grad << std::endl;
            /* Computes the linear strain tensor from the deformation gradient*/
            computeStrain(Grad,strain[i*dim + k]);

          }

        // loop over test function number
        for (int row=0; row<sfs.size(); row++) {

          for (int rcomp=0; rcomp<dim; rcomp++) {

            SymmTensor stress = hookeTimesStrain(strain[row*dim + rcomp]);

            //std::cout << "Stress: " << stress << std::endl;
            for (int col=0; col<=row; col++) {

              for (int ccomp=0; ccomp<dim; ccomp++) {

                this->A[row][col][rcomp][ccomp] += stress*strain[col*dim + ccomp] * weight * detjac;

              }
            }
          }
#if 0
          // rhs
          b[i] += q*sfs[i].evaluateFunction(0,local)*factor;

          // matrix
          gv = 0;
          K.umv(grad[i],gv);         // multiply with diffusion tensor
          A[i][i] += (grad[i]*gv)*factor;
          for (int j=0; j<i; j++)
          {
            RT t = (grad[j]*gv)*factor;
            A[i][j] += t;
            A[j][i] += t;
          }

#endif

        }

      }

#if 0
      for (int row=0; row<sfs.size(); row++)
        for (int col=0; col<=row; col++) {

          // Complete the symmetric matrix by copying the lower left triangular matrix
          // to the upper right one
          if (row!=col) {
            for (int rcomp=0; rcomp<dim; rcomp++)
              for (int ccomp=0; ccomp<dim; ccomp++)
                this->A[col][row][ccomp][rcomp] = this->A[row][col][rcomp][ccomp];
          }
        }
#endif

      std::cout << this->A[0][0] << std::endl;
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

    // computes the linear strain from the deformation gradient
    void computeStrain(const FieldMatrix<double, dim, dim>& grad,
                       SymmTensor& strain) const
    {

      if (dim==2) {

        strain[0] = grad[0][0];
        strain[1] = grad[1][1];
        strain[2] = grad[0][1] + grad[1][0];

      } else if (dim==3) {

        strain[0] = grad[0][0];
        strain[1] = grad[1][1];
        strain[2] = grad[2][2];
        strain[3] = grad[0][1] + grad[1][0];
        strain[4] = grad[0][2] + grad[2][0];
        strain[5] = grad[2][1] + grad[1][2];

      } else
        DUNE_THROW(NotImplemented, "No elasticity assembler for " << dim << "-dimensional problems");
    }


    SymmTensor hookeTimesStrain(const SymmTensor& strain) const {

      if (dim==3) {

        // compute Hooke Tensor
        FieldMatrix<double, 6, 6> hookeTensor;

        hookeTensor = 0;

        hookeTensor[0][0] = 1-nu_;
        hookeTensor[0][1] = nu_;
        hookeTensor[0][2] = nu_;

        hookeTensor[1][0] = nu_;
        hookeTensor[1][1] = 1-nu_;
        hookeTensor[1][2] = nu_;

        hookeTensor[2][0] = nu_;
        hookeTensor[2][1] = nu_;
        hookeTensor[2][2] = 1-nu_;

        hookeTensor[3][3] = 0.5 -nu_;
        hookeTensor[4][4] = 0.5 -nu_;
        hookeTensor[5][5] = 0.5 -nu_;

        hookeTensor *= (E_/(1+nu_)/(1-2*nu_));

        // compute stress
        SymmTensor stress;
        stress = 0;
        hookeTensor.umv(strain, stress);

        return stress;

      } else if (dim==2) {

        // compute Hooke Tensor
        FieldMatrix<double, 3, 3> hookeTensor;

        hookeTensor = 0;

        hookeTensor[0][0] = 1-nu_;
        hookeTensor[0][1] = nu_;

        hookeTensor[1][0] = nu_;
        hookeTensor[1][1] = 1-nu_;

        hookeTensor[2][2] = 0.5 -nu_;

        hookeTensor *= (E_/(1+nu_)/(1-2*nu_));

        // compute stress
        SymmTensor stress;
        stress = 0;
        hookeTensor.umv(strain, stress);

        return stress;

      } else
        DUNE_THROW(NotImplemented, "No elasticity assembler for " << dim << "-dimensional problems");

    }

#if 0
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
#endif


  private:

    // parameters given in constructor
    bool procBoundaryAsDirichlet;

#if 0
    // assembled data
    int currentsize;
    MBlockType A[SIZE][SIZE];
    VBlockType b[SIZE];
    BCBlockType bctype[SIZE];
#endif
  };

  /** @} */
}
#endif
