// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P1_MASSMATRIX_ASSEMBLER_HH
#define DUNE_P1_MASSMATRIX_ASSEMBLER_HH

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"
#include "istl/operators.hh"
#include "istl/bcrsmatrix.hh"

#include "disc/shapefunctions/lagrangeshapefunctions.hh"
#include "disc/operators/p1operator.hh"
#include "disc/operators/boundaryconditions.hh"
#include "disc/functions/p1function.hh"

/**
 * @file
 * @brief  compute local stiffness matrix for conforming finite elements for the
 *         vector values mass matrix
 * @author Oliver Sander
 */


namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief compute local stiffness matrix for conforming finite elements for the mass matrix
   *
        Uses conforming finite elements with the Lagrange shape functions.
        It should work for all dimensions and element types.
        All the numbering is with respect to the reference element and the
        Lagrange shape functions

        Template parameters are:

        - Grid  a DUNE grid type
        - RT    type used for return values
       - comp Number of components of the system
   */
  template<class GridType, class RT, int comp>
  class MassMatrixLocalStiffness
  {
    // grid types
    typedef typename GridType::ctype DT;
    typedef typename GridType::Traits::template Codim<0>::Entity Entity;
    typedef typename GridType::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename GridType::Traits::IntersectionIterator IntersectionIterator;

    // some other sizes
    enum {dim=GridType::dimension};
    enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,dim>::maxsize};

  public:

    // Number of components for the global assembler to collect
    enum {m=comp};

    // types for matrics, vectors and boundary conditions
    typedef FieldMatrix<RT,comp,comp> MBlockType;   // one entry in the stiffness matrix
    typedef FieldVector<RT,comp> VBlockType;     // one entry in the global vectors
    typedef FixedArray<BoundaryConditions::Flags,comp> BCBlockType;       // componentwise boundary conditions

    //! Constructor
    MassMatrixLocalStiffness (bool procBoundaryAsDirichlet_=true) {

      currentsize = 0;
      procBoundaryAsDirichlet = procBoundaryAsDirichlet_;

      // For the time being:  all boundary conditions are homogeneous Neumann
      // This means no boundary condition handling is done at all
      for (int i=0; i<SIZE; i++)
        bctype[i] = BoundaryConditions::neumann;
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
      NewGeometryType gt = e.geometry().type();
      const typename LagrangeShapeFunctionSetContainer<DT,RT,dim>::value_type& sfs
        = LagrangeShapeFunctions<DT,RT,dim>::general(gt,k);

      // clear assemble data
      for (int i=0; i<sfs.size(); i++) {
        b[i] = 0;
        bctype[i][0] = BoundaryConditions::neumann;
        for (int j=0; j<sfs.size(); j++)
          A[i][j] = 0;
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

        // determinant of jacobian
        DT detjac = e.geometry().integrationElement(quadPos);

        RT factor = weight*detjac;


        double v[sfs.size()];
        for(int i=0; i<sfs.size(); i++)
          v[i] = sfs[i].evaluateFunction(0,quadPos);

        for(int i=0; i<sfs.size(); i++)
          for (int j=0; j<=i; j++ )
            for (int k=0; k<comp; k++)
              A[i][j][k][k] += ( v[i] * v[j] ) * factor;

      }

      for (int row=0; row<sfs.size(); row++)
        for (int col=0; col<=row; col++) {

          // Complete the symmetric matrix by copying the lower left triangular matrix
          // to the upper right one
          if (row!=col) {
            for (int rcomp=0; rcomp<dim; rcomp++)
              for (int ccomp=0; ccomp<dim; ccomp++)
                A[col][row][ccomp][rcomp] = A[row][col][rcomp][ccomp];
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
