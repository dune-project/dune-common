// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1GROUNDWATER_HH
#define DUNE_P1GROUNDWATER_HH

#include <map>
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
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename G::Traits::IntersectionIterator IntersectionIterator;
    enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize, SIZEF=SIZE};

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

      //          std::cout << "Entity:" << std::endl;
      //          for (int i=0; i<e.template count<n>(); i++)
      //                std::cout << "corner i=" << i << " pos=" << e.geometry()[i] << std::endl;

      // clear assemble data
      for (int i=0; i<sfs.size(); i++)
      {
        b[i] = 0;
        bctype[i] = GroundwaterEquationParameters<G,RT>::neumann;
        for (int j=0; j<sfs.size(); j++)
          A[i][j] = 0;
      }

      // Loop over all quadrature points and assemble matrix and right hand side
      int p=1;
      if (k>1) p=2*(k-1);
      for (int g=0; g<Dune::QuadratureRules<DT,n>::rule(gt,p).size(); ++g)     // run through all quadrature points
      {
        const Dune::FieldVector<DT,n>& local = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].position();           // pos of integration point
        Dune::FieldVector<DT,n> global = e.geometry().global(local);                                            // ip in global coordinates
        const Dune::FieldMatrix<DT,n,n> jac = e.geometry().jacobianInverseTransposed(local);                             // eval jacobian inverse
        const Dune::FieldMatrix<DT,n,n> K = problem.K(global,e,local);                                         // eval diffusion tensor
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
            FieldVector<DT,n> global = it.intersectionGlobal().global(facelocal);
            bctypeface = problem.bctype(global,e,local);                       // eval bctype

            //                            std::cout << "=== Boundary found"
            //                                                  << " facenumber=" << it.numberInSelf()
            //                                                  << " quadpoint=" << g
            //                                                  << " facelocal=" << facelocal
            //                                                  << " local=" << local
            //                                                  << " global=" << global
            //                                                  << std::endl;

            if (bctypeface!=GroundwaterEquationParameters<G,RT>::neumann) break;

            RT J = problem.J(global,e,local);
            double weightface = Dune::QuadratureRules<DT,n-1>::rule(gtface,p)[g].weight();
            DT detjacface = it.intersectionGlobal().integrationElement(facelocal);
            for (int i=0; i<sfs.size(); i++)                       // loop over test function number
              if (bctype[i]==GroundwaterEquationParameters<G,RT>::neumann)
              {
                b[i] -= J*sfs[i].evaluateFunction(0,local)*weightface*refvolumeface*detjacface;
                //                                              std::cout << "Neumann BC found, accumulating"
                //                                                                << -J*sfs[i].evaluateFunction(0,local)*weightface*refvolumeface*detjacface
                //                                                                << std::endl;
                //                                              std::cout << "J=" << J << " shapef=" << sfs[i].evaluateFunction(0,local)
                //                                                                << " weight=" << weightface
                //                                                                << " refvolume=" << refvolumeface
                //                                                                << " detjac=" << detjacface << std::endl;
              }
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
            if (sfs[i].entity()==ReferenceElements<DT,n>::general(gt).subEntity(it.numberInSelf(),1,j,sfs[i].codim()))
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
    RT& mat (int i, int j)
    {
      return A[i][j];
    }

    //! access right hand side
    /*! Access elements of the right hand side vector. Elements are
       undefined without prior call to the assemble method.
     */
    RT& rhs (int i)
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


    /** \brief Evaluate element part of error estimator

       This functions is only implemented for P1 elements !
            This function assumes a conforming mesh !

        @param[in]  e    a codim 0 entity reference
        @param[out]  elementpart    element part of error estimator

     */
    void estimate (const Entity& e, RT& elementpart)
    {
      // extract some important parameters
      Dune::GeometryType gt = e.geometry().type();
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);
      DT Zero = 0;
      double refvolume = Dune::ReferenceElements<DT,n>::general(gt).volume();
      Dune::FieldVector<DT,n> center = e.geometry().global(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));

      // integral over right hand side, div(K grad u_h) = 0 for P1 elements
      int p=1;
      RT volume = e.geometry().integrationElement(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));
      RT h_K = pow(volume,1.0/((double)n));
      RT integralq = 0;
      for (int g=0; g<Dune::QuadratureRules<DT,n>::rule(gt,p).size(); ++g)     // run through all quadrature points
      {
        const Dune::FieldVector<DT,n>& local = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].position();           // pos of integration point
        Dune::FieldVector<DT,n> global = e.geometry().global(local);                                            // ip in global coordinates
        double weight = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].weight();                                    // weight of quadrature point
        DT detjac = e.geometry().integrationElement(local);                                                     // determinant of jacobian
        RT q = problem.q(global,e,local);           // source term
        integralq += q*q*weight*refvolume*detjac;
      }
      integralq *= h_K*h_K;     // scaling by h_K^2
      elementpart = integralq;
    }

    /** \brief Evaluate face part of error estimator for a given face

       This functions is only implemented for P1 elements !
            This function assumes a conforming mesh !

        @param[in]  e    a codim 0 entity reference
        @param[in]  it    an intersection iterator started from e
        @param[out]  facefluxK  the faceflux is evaluated as Sum facefluxK[i]*coeff[i] with coefficients of finite element function
        @param[out]  facefluxN  same for neighbor of the given face
            @param[out]  facefactor factor by which difference of fluxes has to be multiplied
            @param[out]  facebctype if bctype is neumann then facefluxN[0] contains neumann value
     */
    void estimate (const Entity& e, const IntersectionIterator& it, const EEntityPointer& outside,
                   RT facefluxK[], RT facefluxN[], RT& facefactor,
                   typename GroundwaterEquationParameters<G,RT>::BC& facebctype, bool first)
    {

      // extract some important parameters
      GeometryType gt = e.geometry().type();
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);
      DT Zero = 0;
      double refvolume = Dune::ReferenceElements<DT,n>::general(gt).volume();
      Dune::FieldVector<DT,n> center = e.geometry().global(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));

      // the edge term
      GeometryType gtface = it.intersectionSelfLocal().type();
      double refvolumeface = Dune::ReferenceElements<DT,n-1>::general(gtface).volume();
      int numberInSelf = it.numberInSelf();
      const Dune::FieldVector<DT,n-1>& facelocal = Dune::ReferenceElements<DT,n-1>::general(gtface).position(0,0);
      FieldVector<DT,n> local = it.intersectionSelfLocal().global(facelocal);
      FieldVector<DT,n> global = it.intersectionGlobal().global(facelocal);
      FieldVector<DT,n> unitOuterNormal = it.unitOuterNormal(facelocal);

      // compute face factor
      FieldMatrix<DT,n,n> jac;
      FieldMatrix<DT,n,n> Kjac;
      DT detjacface = it.intersectionGlobal().integrationElement(facelocal);
      DT h_e = pow(detjacface,1.0/((double)n-1));
      facefactor = detjacface*h_e;

      // compute Kgradphi
      if (first || gt!=simplex)
      {
        jac = e.geometry().jacobianInverseTransposed(local);           // eval jacobian inverse at face center
        Kjac = problem.K(center,e,local);                       // eval diffusion tensor at face center
        Kjac.rightmultiply(jac);
        for (int i=0; i<sfs.size(); i++)
        {
          FieldVector<DT,n> temp;
          for (int l=0; l<n; l++)
            temp[l] = sfs[i].evaluateDerivative(0,l,local);
          cache[i] = 0;
          Kjac.umv(temp,cache[i]);                 // multiply with diffusion tensor
        }
      }

      // handle interior edge
      if (it.neighbor())
      {
        // no neumann condition
        facebctype = GroundwaterEquationParameters<G,RT>::process;

        // compute coefficients of flux evaluation in self
        for (int i=0; i<sfs.size(); i++)
        {
          facefluxK[i] = -(cache[i]*unitOuterNormal);
        }

        // compute coefficients of flux evaluation in neighbor
        GeometryType nbgt = outside->geometry().type();
        FieldVector<DT,n> nbcenter = outside->geometry().global(ReferenceElements<DT,n>::general(nbgt).position(0,0));
        const typename LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& nbsfs=LagrangeShapeFunctions<DT,RT,n>::general(nbgt,1);
        GeometryType nbgtface = it.intersectionNeighborLocal().type();
        double nbrefvolumeface = ReferenceElements<DT,n-1>::general(nbgtface).volume();
        int numberInNeighbor = it.numberInNeighbor();
        const FieldVector<DT,n-1>& nbfacelocal = ReferenceElements<DT,n-1>::general(nbgtface).position(0,0);
        FieldVector<DT,n> nblocal = it.intersectionNeighborLocal().global(nbfacelocal);
        FieldMatrix<DT,n,n> nbjac = outside->geometry().jacobianInverseTransposed(nblocal);
        FieldMatrix<DT,n,n> nbKjac = problem.K(nbcenter,*outside,nblocal);
        nbKjac.rightmultiply(nbjac);
        for (int i=0; i<nbsfs.size(); i++)
        {
          FieldVector<DT,n> temp;
          for (int l=0; l<n; l++)
            temp[l] = nbsfs[i].evaluateDerivative(0,l,nblocal);
          FieldVector<DT,n> Kgradphi(0);
          nbKjac.umv(temp,Kgradphi);                 // multiply with diffusion tensor
          facefluxN[i] = -(Kgradphi*unitOuterNormal);
        }
        return;
      }

      // handle face on exterior boundary Neumann boundary
      if (it.boundary())
      {
        // evaluate boundary condition type
        facebctype = problem.bctype(global,e,local);
        if (facebctype!=GroundwaterEquationParameters<G,RT>::neumann)
          return;               // only Neumann conditions require further work

        // evaluate Neumann boundary
        facefluxN[0] = problem.J(global,e,local);

        // compute coefficients of flux evaluation in self
        for (int i=0; i<sfs.size(); i++)
        {
          facefluxK[i] = -(cache[i]*unitOuterNormal);
        }
        return;
      }
    }

  private:
    // local stiffness matrix
    int currentsize;
    const GroundwaterEquationParameters<G,RT>& problem;
    RT A[SIZE][SIZE];
    RT b[SIZE];
    typename GroundwaterEquationParameters<G,RT>::BC bctype[SIZE];
    bool procBoundaryAsDirichlet;
    FieldVector<DT,n> cache[SIZE];
  };


  //! The global stiffness matrix
  template<class G, class RT, class IS>
  class P1GroundwaterOperator : public AssembledP1FEOperator<G,RT,IS>
  {
    typedef typename G::ctype DT;
    enum {n=G::dimension};
    typedef typename G::template Codim<0>::Entity Entity;
    typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
    typedef typename G::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename G::template Codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;
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
    void assemble (P1FEFunction<G,RT,IS>& u, P1FEFunction<G,RT,IS>& f)
    {
      // clear global stiffness matrix and right hand side
      this->A = 0;
      *f = 0;

      // allocate flag vector to hold flags for essential boundary conditions
      std::vector<unsigned char> essential(this->vertexmapper.size());
      for (int i=0; i<essential.size(); i++) essential[i] = GroundwaterEquationParameters<G,RT>::neumann;

      // allocate flag vector to note hanging nodes whose row has been assembled
      std::vector<unsigned char> treated(this->vertexmapper.size());
      for (int i=0; i<treated.size(); i++) treated[i] = false;

      // hanging node stuff
      RT alpha[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize][Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];

      // local to global id mapping (do not ask vertex mapper repeatedly
      int l2g[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
      int fl2g[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];

      // run over all leaf elements
      Iterator eendit = this->is.template end<0,All_Partition>();
      for (Iterator it = this->is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        // get access to shape functions for P1 elements
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type&
        sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);

        // get local to global id map
        for (int k=0; k<sfs.size(); k++)
        {
          if (sfs[k].codim()!=n) DUNE_THROW(MathError,"expected codim==dim");
          int alpha = this->vertexmapper.template map<n>(*it,sfs[k].entity());
          l2g[k] = alpha;
        }

        // build local stiffness matrix for P1 elements
        // inludes rhs and boundary condition information
        loc.assemble(*it,1);           // assemble local stiffness matrix

        // assemble constraints in hanging nodes
        // as a by-product determine the interpolation factors WITH RESPECT TO NODES OF FATHER
        bool hasHangingNodes = false;
        for (int k=0; k<sfs.size(); k++)           // loop over rows, i.e. test functions
        {
          // process only hanging nodes
          if (!this->hanging[l2g[k]]) continue;
          hasHangingNodes = true;

          // determine position of hanging node in father
          EEntityPointer father=it->father();                 // the father element
          GeometryType gtf = father->geometry().type();                 // fathers type
          assert(gtf==gt);                 // in hanging node refinement the element type is preserved
          const FieldVector<DT,n>& cpos=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[k].position();
          FieldVector<DT,n> pos = it->geometryInFather().global(cpos);                 // map corner to father element

          // evaluate interpolation factors and local to global mapping for father
          for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1).size(); ++i)
          {
            alpha[i][k] = Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1)[i].evaluateFunction(0,pos);
            fl2g[i] = this->vertexmapper.template map<n>(*father,i);
          }

          // assemble the constraint row once
          if (!treated[l2g[k]])
          {
            bool throwflag=false;
            int cnt=0;
            for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1).size(); ++i)
              if (std::abs(alpha[i][k])>1E-4)
              {
                if (this->hanging[fl2g[i]])
                {
                  std::cout << "X i=" << father->geometry()[i]
                            << " k=" << it->geometry()[k]
                            << " alpha=" << alpha[i][k]
                            << " cnt=" << ++cnt
                            << std::endl;
                  throwflag = true;
                }
                this->A[l2g[k]][fl2g[i]] = -alpha[i][k];
              }
            if (false)
              DUNE_THROW(GridError,"hanging node interpolated from hanging node");
            this->A[l2g[k]][l2g[k]] = 1;
            (*f)[l2g[k]] = 0;
            treated[l2g[k]] = true;
          }
        }

        // accumulate local matrix into global matrix for non-hanging nodes
        for (int i=0; i<sfs.size(); i++)           // loop over rows, i.e. test functions
        {
          // process only non-hanging nodes
          if (this->hanging[l2g[i]]) continue;

          // accumulate matrix
          for (int j=0; j<sfs.size(); j++)
          {
            // process only non-hanging nodes
            if (this->hanging[l2g[j]]) continue;

            // the standard entry
            this->A[l2g[i]][l2g[j]] += loc.mat(i,j);
          }

          // accumulate essential boundary condition
          if (loc.bc(i)>essential[l2g[i]])
          {
            essential[l2g[i]] = loc.bc(i);
            (*f)[l2g[i]] = loc.rhs(i);
          }

          // accumulate rhs
          if (essential[l2g[i]]==GroundwaterEquationParameters<G,RT>::neumann)
            (*f)[l2g[i]] += loc.rhs(i);
        }

        // add corrections for hanging nodes
        if (hasHangingNodes)
        {
          // a map that inverts l2g
          std::map<int,int> g2l;
          for (int i=0; i<sfs.size(); i++)
            g2l[l2g[i]] = i;

          // a map that inverts fl2g
          std::map<int,int> fg2l;
          for (int i=0; i<sfs.size(); i++)
            fg2l[fl2g[i]] = i;

          EEntityPointer father=it->father();                 // DEBUG

          // loop over nodes (rows) in father, assume father has same geometry type
          for (int i=0; i<sfs.size(); ++i)
          {
            // corrections to matrix
            // loop over all nodes (columns) in father, assume father has same geometry type
            for (int j=0; j<sfs.size(); ++j)
            {
              // loop over hanging nodes
              for (int k=0; k<sfs.size(); k++)
              {
                // process only hanging nodes
                if (!this->hanging[l2g[k]]) continue;

                // first term, with i a coarse vertex
                if ( std::abs(alpha[j][k])>1E-4 && g2l.find(fl2g[i])!=g2l.end() )
                {
                  //                                                      std::cout << "term 1a"
                  //                                                                            << " i=" << father->geometry()[i]
                  //                                                                            << " j=" << father->geometry()[j]
                  //                                                                            << " k=" << it->geometry()[k]
                  //                                                                            << std::endl;
                  this->A[fl2g[i]][fl2g[j]] += alpha[j][k]*loc.mat(g2l[fl2g[i]],k);
                }

                // first term, with i a fine non-hanging vertex
                if ( std::abs(alpha[j][k])>1E-4 && fg2l.find(l2g[i])==fg2l.end() && !this->hanging[l2g[i]])
                {
                  //                                                      std::cout << "term 1b"
                  //                                                                            << " i=" << it->geometry()[i]
                  //                                                                            << " j=" << father->geometry()[j]
                  //                                                                            << " k=" << it->geometry()[k]
                  //                                                                            << " gi=" << l2g[i] << " gj=" << fl2g[j]
                  //                                                                            << std::endl;
                  this->A[l2g[i]][fl2g[j]] += alpha[j][k]*loc.mat(i,k);
                }

                // second term, with j a coarse vertex
                if ( std::abs(alpha[i][k])>1E-4 && g2l.find(fl2g[j])!=g2l.end() )
                {
                  //                                                      std::cout << "term 2a"
                  //                                                                            << " i=" << father->geometry()[i]
                  //                                                                            << " j=" << father->geometry()[j]
                  //                                                                            << " k=" << it->geometry()[k]
                  //                                                                            << std::endl;
                  this->A[fl2g[i]][fl2g[j]] += alpha[i][k]*loc.mat(k,g2l[fl2g[j]]);
                }

                // second term, with j a fine non-hanging vertex
                if ( std::abs(alpha[i][k])>1E-4 && fg2l.find(l2g[j])==fg2l.end() && !this->hanging[l2g[j]])
                {
                  //                                                      std::cout << "term 2b"
                  //                                                                            << " i=" << father->geometry()[i]
                  //                                                                            << " j=" << it->geometry()[j]
                  //                                                                            << " k=" << it->geometry()[k]
                  //                                                                            << std::endl;
                  this->A[fl2g[i]][l2g[j]] += alpha[i][k]*loc.mat(k,j);
                }

                // third term, loop over hanging nodes
                for (int l=0; l<sfs.size(); l++)
                {
                  // process only hanging nodes
                  if (!this->hanging[l2g[l]]) continue;

                  if ( std::abs(alpha[i][k])>1E-4 && std::abs(alpha[j][l])>1E-4 )
                  {
                    //                                                            std::cout << "term 3"
                    //                                                                                  << " i=" << father->geometry()[i]
                    //                                                                                  << " j=" << father->geometry()[j]
                    //                                                                                  << " k=" << it->geometry()[k]
                    //                                                                                  << " l=" << it->geometry()[l]
                    //                                                                                  << std::endl;
                    this->A[fl2g[i]][fl2g[j]] += alpha[i][k]*alpha[j][l]*loc.mat(k,l);
                  }
                }
              }
            }

            // corrections to rhs
            if (essential[fl2g[i]]==GroundwaterEquationParameters<G,RT>::neumann)
              for (int k=0; k<sfs.size(); k++)
              {
                // process only hanging nodes
                if (!this->hanging[l2g[k]]) continue;

                if ( std::abs(alpha[i][k])>1E-4 && loc.bc(k)==GroundwaterEquationParameters<G,RT>::neumann )
                  (*f)[fl2g[i]] += alpha[i][k]*loc.rhs(k);
              }
          }
        }
      }


      // put in essential boundary conditions
      rowiterator endi=this->A.end();
      for (rowiterator i=this->A.begin(); i!=endi; ++i)
      {
        if (!this->hanging[i.index()] && essential[i.index()]!=GroundwaterEquationParameters<G,RT>::neumann)
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

    /** \brief evaluate error estimator
     */
    void estimate (const P1FEFunction<G,RT,IS>& u, P0FEFunction<G,RT,IS>& eta2)
    {
      // clear estimator values
      *eta2 = 0;

      // run over all leaf elements
      Iterator eendit = this->is.template end<0,All_Partition>();
      for (Iterator it = this->is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        // in case someone calls it with a level index set
        assert(it->isLeaf());

        // get access to shape functions for P1 elements
        Dune::GeometryType gt = it->geometry().type();
        const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type&
        sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);

        // evaluate element part of estimator
        RT elementpart;
        loc.estimate(*it,elementpart);
        (*eta2)[eta2.mapper().map(*it)] += elementpart;


        // loop over all neighbors
        IntersectionIterator iendit = it->iend();
        bool first=true;
        for (IntersectionIterator iit = it->ibegin(); iit!=iendit; ++iit)
        {
          // handle face with neighbor
          if (iit.neighbor())
          {
            // Avoid calling the outside() method often
            // it is extremely expensive !
            const EEntityPointer outside = iit.outside();

            // if neighbor is not leaf then it is evaluated on the neighbor
            if (!outside->isLeaf())
              continue;

            // check if face is handled from other side
            if (outside->level()==it->level() && eta2.mapper().map(*it)<eta2.mapper().map(*outside))
              continue;

            // evaluate coefficients for this face
            RT facefluxK[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
            RT facefluxN[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
            RT facefactor;
            typename GroundwaterEquationParameters<G,RT>::BC facebctype;
            loc.estimate(*it,iit,outside,facefluxK,facefluxN,facefactor,facebctype,first);
            first=false;

            // compute contribution of myself
            RT self=0;
            for (int i=0; i<it->template count<n>(); i++)
              self += facefluxK[i]*(*u)[u.mapper().template map<n>(*it,i)];

            // compute contribution of nb
            RT nb=0;
            for (int i=0; i<outside->template count<n>(); i++)
              nb += facefluxN[i]*(*u)[u.mapper().template map<n>(*outside,i)];


            // accumulate contribution to both elements
            (*eta2)[eta2.mapper().map(*it)] += 0.5*facefactor*(self-nb)*(self-nb);
            (*eta2)[eta2.mapper().map(*outside)] += 0.5*facefactor*(self-nb)*(self-nb);

            continue;
          }

          // handle face on boundary
          if (iit.boundary())
          {
            // evaluate coefficients for this face
            RT facefluxK[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
            RT facefluxN[Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize];
            RT facefactor;
            typename GroundwaterEquationParameters<G,RT>::BC facebctype;
            loc.estimate(*it,iit,iit.inside(),facefluxK,facefluxN,facefactor,facebctype,first);
            first=false;

            // check bc type
            if (facebctype!=GroundwaterEquationParameters<G,RT>::neumann)
              continue;

            // compute contribution of myself
            RT self=0;
            for (int i=0; i<it->template count<n>(); i++)
              self += facefluxK[i]*(*u)[u.mapper().template map<n>(*it,i)];

            // accumulate contribution
            (*eta2)[eta2.mapper().map(*it)] += facefactor*(facefluxN[0]-self)*(facefluxN[0]-self);

            continue;
          }
        }

      }
    }

    void preMark ()
    {
      marked.resize(this->vertexmapper.size());
      for (int i=0; i<marked.size(); i++) marked[i] = false;
      return;
    }

    void postMark (G& g)
    {
      // run over all leaf elements
      Iterator eendit = this->is.template end<0,All_Partition>();
      for (Iterator it = this->is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        // get access to shape functions for P1 elements
        Dune::GeometryType gt = it->geometry().type();
        if (gt!=Dune::simplex && gt!=Dune::triangle && gt!=Dune::tetrahedron) continue;

        const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type&
        sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);

        // count nodes with mark
        int count=0;
        for (int k=0; k<sfs.size(); k++)
        {
          int alpha = this->vertexmapper.template map<n>(*it,sfs[k].entity());
          if (marked[alpha]) count++;
        }

        // refine if a marked edge exists
        if (count>1) {
          std::cout << "extra mark" << std::endl;
          g.mark(1,it);
        }
      }

      marked.clear();
      return;
    }

    void mark (G& g, EEntityPointer& it)
    {
      // refine this element
      g.mark(1,it);

      // check geom type, exit if not simplex
      Dune::GeometryType gt = it->geometry().type();
      if (gt!=Dune::simplex && gt!=Dune::triangle && gt!=Dune::tetrahedron) return;
      assert(it->isLeaf());

      // determine if element has hanging nodes
      bool hasHangingNodes = false;
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type&
      sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);
      for (int k=0; k<sfs.size(); k++)     // loop over rows, i.e. test functions
        if (this->hanging[this->vertexmapper.template map<n>(*it,sfs[k].entity())])
        {
          hasHangingNodes = true;
          break;
        }

      // if no hanging nodes we are done
      if (!hasHangingNodes) return;

      // mark all corners of father
      EEntityPointer father=it->father();
      for (int k=0; k<sfs.size(); k++)     // same geometry type ...
      {
        int alpha=this->vertexmapper.template map<n>(*father,k);
        marked[alpha] = true;
      }

      return;
    }


  private:
    std::vector<bool> marked;
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
