// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
# include "config.h"     // autoconf defines, needed by the dune headers
#endif
#include "dgstokes.hh"
#include "testfunctions.hh"




template<class G,int ordr>
void DGStokes<G,ordr>::assembleVolumeTerm(Entity& epointer, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const
{
  Gradient grad_phi_ei[2],grad_phi_ej[2],temp;
  ctype phi_ei, phi_ej,psi_ei,psi_ej;
  ctype entry;

  //get the shape function set
  ShapeFunctionSet vsfs(ordr);; //for  velocity
  ShapeFunctionSet psfs(ordr-1); // for pressure
  //shape function size and total dof
  int vdof=vsfs.size()*2; // two velocity components and total velocity sfs size
  int pdof=psfs.size();
  //get parameter
  DGStokesParameters parameter;

  //get the geometry type
  Dune::GeometryType gt = *epointer.geometry().type();
  //specify the quadrature order ?
  int qord=18;
  for (int nqp=0; nqp<Dune::QuadratureRules<ctype,dim>::rule(gt,qord).size(); ++nqp)
  {
    //local position of quad points
    const Dune::FieldVector<ctype,dim> & quad_point_loc = Dune::QuadratureRules<ctype,2>::rule(gt,qord)[nqp].position();
    //global position
    Dune::FieldVector<ctype,dim> quad_point_glob = *epointer.geometry().global(quad_point_loc);
    // calculate inv jacobian
    InverseJacobianMatrix inv_jac=*epointer.geometry().jacobianInverseTransposed(quad_point_loc);
    // quadrature weight
    double quad_wt=Dune::QuadratureRules<ctype,dim>::rule(gt,qord)[nqp].weight();
    // get the determinant jacobian
    ctype detjac=*epointer.geometry().integrationElement(quad_point_loc);


    //================================================//
    // source term: TERM:14 : f* v
    // TERM 1 : \int (mu*grad_u*grad_v)
    //================================================//
    // dimension - 2 velocity comps. (now assumming dim==2)
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        //space dimension-sd  (now assumed dim==2)
        for (int sd=0; sd<2; sd++)
          temp[sd] = vsfs[i].evaluateDerivative(0,sd,quad_point_loc);
        grad_phi_ei[dm-1] = 0;
        //matrix vect multiplication
        // transform gradient to global coordinates by multiplying with inverse jacobian
        inv_jac.umv(temp,grad_phi_ei[dm-1]);
        int ii=(dm-1)*vsfs.size()+i;
        double f;
        RHS rhs;
        if (dm==1)
        {
          rhs.u_rhs(quad_point_glob[0],quad_point_glob[1],f);
        }
        else
        {
          rhs.v_rhs(quad_point_glob[0],quad_point_glob[1],f);
        }
        Be[ii]+=f*vsfs[i].evaluateFunction(0,quad_point_loc)*detjac*quad_wt;

        for (int j=0; j<vsfs.size(); ++j)
        {
          for (int sd=0; sd<2; sd++)                        //space dimension -sd
            temp[sd] = vsfs[j].evaluateDerivative(0,sd,quad_point_loc);
          grad_phi_ej[dm-1] = 0;
          inv_jac.umv(temp,grad_phi_ej[dm-1]);
          int jj=(dm-1)*vsfs.size()+j;
          entry =parameter.mu*(grad_phi_ei[dm-1]*grad_phi_ej[dm-1])*detjac*quad_wt;
          Aee.add(ii,jj,entry);
        }
      }
    }
    //================================================//
    // -  p * div v
    //================================================//
    for(int dm=1; dm<=2; ++dm)    //(now assumming dim==2)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        for (int sd=0; sd<2; sd++)                  //space dimension -sd
          temp[sd] = vsfs[i].evaluateDerivative(0,sd,quad_point_loc);
        grad_phi_ei[dm-1] = 0;
        inv_jac.umv(temp,grad_phi_ei[dm-1]);
        for (int j=0; j<psfs.size(); ++j)                 // pressure shapefns
        {
          int jj=vdof+j;
          psi_ej=psfs[j].evaluateFunction(0,quad_point_loc);
          entry =-(grad_phi_ei[dm-1][dm-1]*psi_ej)*detjac*quad_wt;
          Aee.add(ii,jj,entry);
        }
      }
    }
    //================================================//
    //   -  q* div u
    //================================================//
    for(int dm=1; dm<=2; ++dm)    //(now assumming dim==2)
    {
      for (int i=0; i<psfs.size(); ++i)           // pressure shapefns
      {
        int ii=vdof+i;
        psi_ei=psfs[i].evaluateFunction(0,quad_point_loc);
        //if(i==0) psi_ei=0.0;
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          for (int sd=0; sd<2; sd++)                        //space dimension -sd
            temp[sd] = vsfs[j].evaluateDerivative(0,sd,quad_point_loc);
          grad_phi_ej[dm-1] = 0;
          inv_jac.umv(temp,grad_phi_ej[dm-1]);
          entry =-(grad_phi_ej[dm-1][dm-1]*psi_ei)*detjac*quad_wt;
          Aee.add(ii,jj,entry);
          //std::cout<<"AA(): "<<AA(ig,jg)<<std::endl;
        }
      }
    }
    //================================================//

  }       // end of volume term quadrature
} // end of assemble volume term









template<class G,int ordr>
void DGStokes<G,ordr>::assembleStokesSystem()
{
  ShapeFunctionSet vsfs(ordr);; //for  velocity
  ShapeFunctionSet psfs(ordr-1); // for pressure

  int vdof=vsfs.size()*2; // two velocity components and total velocity sfs size
  int pdof=psfs.size();
  int ndof=vdof+pdof; // total dofs per element
  int N = ndof*grid.size(level, 0);
  int nz=N;
  DGStokesParameters parameter;
  Dune::SparseRowMatrix<double> AA(N,N,nz);
  Dune::SimpleVector<double> bb(N);
  bb=0.0;
  // loop over all elements
  ElementIterator it = grid.template lbegin<0>(level);
  ElementIterator itend = grid.template lend<0>(level);
  for (; it != itend; ++it)
  {
    EntityPointer epointer = it;
    int eid = grid.levelIndexSet(level).index(*epointer);
    // Dune::GeometryType gt = *epointer.geometry().type();
    // void assembleVolumeTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const;
    //  void assembleFaceTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const;
    //  void assembleBoundaryTerm(Entity& ep, LocalMatrixBlock& Aee,LocalVectorBlock& Be)const ;

  }


} // end of assemble
