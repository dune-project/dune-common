// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P1GROUNDWATERESTIMATOR_HH
#define DUNE_P1GROUNDWATERESTIMATOR_HH

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
#include "disc/operators/boundaryconditions.hh"
#include "disc/functions/p0function.hh"
#include "disc/functions/p1function.hh"
#include "groundwater.hh"

/**
 * @file dune/disc/groundwater/p1groundwaterestimator.hh
 * @brief  compute local stiffness matrix for conforming finite elements for diffusion equation
 * @author Peter Bastian
 */


namespace Dune
{
  /** @addtogroup DISC_Disc
   *
   * @{
   */
  /**
   * @brief compute local stiffness matrix for conforming finite elements for diffusion equation
   *
   */

  template<class G, class RT>
  class ElementGroundwaterEstimator {
    typedef typename G::ctype DT;
    enum {n=G::dimension, m=1};
    typedef typename G::Traits::template Codim<0>::Entity Entity;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;
    typedef typename G::Traits::IntersectionIterator IntersectionIterator;
    enum {SIZE=Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::maxsize, SIZEF=SIZE};

  public:
    //! Constructor
    ElementGroundwaterEstimator (const GroundwaterEquationParameters<G,RT>& params)
      : problem(params)
    {}


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
      Dune::FieldVector<DT,n> center = e.geometry().global(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));

      // integral over right hand side, div(K grad u_h) = 0 for P1 elements
      int p=1;
      RT volume = e.geometry().integrationElement(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));
      RT h_K = pow(volume,1.0/((double)n));
      RT integralq = 0;
      for (std::size_t g=0; g<Dune::QuadratureRules<DT,n>::rule(gt,p).size(); ++g)     // run through all quadrature points
      {
        const Dune::FieldVector<DT,n>& local = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].position();           // pos of integration point
        Dune::FieldVector<DT,n> global = e.geometry().global(local);                                            // ip in global coordinates
        double weight = Dune::QuadratureRules<DT,n>::rule(gt,p)[g].weight();                                    // weight of quadrature point
        DT detjac = e.geometry().integrationElement(local);                                                     // determinant of jacobian
        RT q = problem.q(global,e,local);           // source term
        integralq += q*q*weight*detjac;
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
                   typename BoundaryConditions::Flags& facebctype, bool first)
    {

      // extract some important parameters
      GeometryType gt = e.geometry().type();
      const typename Dune::LagrangeShapeFunctionSetContainer<DT,RT,n>::value_type& sfs=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1);
      Dune::FieldVector<DT,n> center = e.geometry().global(Dune::ReferenceElements<DT,n>::general(gt).position(0,0));

      // the edge term
      GeometryType gtface = it.intersectionSelfLocal().type();
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
      if (first || !gt.isSimplex())
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
        facebctype = BoundaryConditions::process;

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
        if (facebctype!=BoundaryConditions::neumann)
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
    const GroundwaterEquationParameters<G,RT>& problem;
    FieldVector<DT,n> cache[SIZEF];;

  };


  //! The global stiffness matrix
  template<class G, class RT>
  class GroundwaterEstimator
  {
    typedef typename G::Traits::LeafIndexSet IS;
    typedef typename G::ctype DT;
    enum {n=G::dimension};
    typedef typename G::template Codim<0>::Entity Entity;
    typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
    typedef typename IS::template Codim<n>::template Partition<All_Partition>::Iterator VIterator;
    typedef typename G::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename G::template Codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;

  public:
    GroundwaterEstimator (const G& grid, const GroundwaterEquationParameters<G,RT>& params)
      :     loc(params),g(grid),is(grid.leafIndexSet())
    {}

    /** \brief evaluate error estimator
     */
    void estimate (const LeafP1Function<G,RT,1>& u, LeafP0Function<G,RT,1>& eta2)
    {
      // clear estimator values
      *eta2 = 0;

      // run over all leaf elements
      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        // in case someone calls it with a level index set
        assert(it->isLeaf());

        // get access to shape functions for P1 elements
        Dune::GeometryType gt = it->geometry().type();

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
            BoundaryConditions::Flags facebctype;
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
            BoundaryConditions::Flags facebctype;
            loc.estimate(*it,iit,iit.inside(),facefluxK,facefluxN,facefactor,facebctype,first);
            first=false;

            // check bc type
            if (facebctype!=BoundaryConditions::neumann)
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

  private:
    ElementGroundwaterEstimator<G,RT> loc;
    const G& g;
    const IS& is;
  };

  /** @} */
}
#endif
