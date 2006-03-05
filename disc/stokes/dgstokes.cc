// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
# include "config.h"     // autoconf defines, needed by the dune headers
#endif
#include "dgstokes.hh"
#include "testfunctions.hh"




template<class G,int ordr>
void DGFiniteElementMethod<G,ordr>::assembleVolumeTerm(Entity& ent, LocalMatrixBlock& Aee,LocalVectorBlock& Be) const
{
  Gradient grad_phi_ei[dim],grad_phi_ej[dim],temp;
  ctype psi_ei,psi_ej;
  ctype entry;

  //get the shape function set
  //for  velocity
  ShapeFunctionSet vsfs(ordr);
  // for pressure
  ShapeFunctionSet psfs(ordr-1);

  //shape function size and total dof
  int vdof=vsfs.size()*dim; // dim velocity components

  //get parameter
  DGStokesParameters parameter;

  //get the geometry type
  Dune::GeometryType gt = ent.geometry().type();
  //specify the quadrature order ?
  int qord=18;
  for (int nqp=0; nqp<Dune::QuadratureRules<ctype,dim>::rule(gt,qord).size(); ++nqp)
  {
    //local position of quad points
    const Dune::FieldVector<ctype,dim> & quad_point_loc = Dune::QuadratureRules<ctype,2>::rule(gt,qord)[nqp].position();
    //global position
    Dune::FieldVector<ctype,dim> quad_point_glob = ent.geometry().global(quad_point_loc);
    // calculate inv jacobian
    InverseJacobianMatrix inv_jac=ent.geometry().jacobianInverseTransposed(quad_point_loc);
    // quadrature weight
    double quad_wt=Dune::QuadratureRules<ctype,dim>::rule(gt,qord)[nqp].weight();
    // get the determinant jacobian
    ctype detjac=ent.geometry().integrationElement(quad_point_loc);


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
        // get the rhs value
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
          Aee[ii][jj]+=entry;

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
          //Aee.add(ii,jj,entry);
          Aee[ii][jj]+=entry;
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
          //Aee.add(ii,jj,entry);
          Aee[ii][jj]+=entry;
        }
      }
    }
    //================================================//

  }       // end of volume term quadrature
} // end of assemble volume term



template<class G,int ordr>
void DGFiniteElementMethod<G,ordr>::assembleFaceTerm(Entity& ent, IntersectionIterator& isit,
                                                     LocalMatrixBlock& Aee, LocalMatrixBlock& Aef,
                                                     LocalMatrixBlock& Afe, LocalVectorBlock& Be) const
{
  Gradient grad_phi_ei[dim],grad_phi_ej[dim],temp;
  ctype phi_ei[dim],phi_ej[dim],phi_fi[dim],phi_fj[dim],psi_ei,psi_ej;
  ctype entry;
  //get the shape function set
  //self shape functions
  ShapeFunctionSet vsfs(ordr);; //for  velocity
  ShapeFunctionSet psfs(ordr-1); // for pressure
  //neighbor shape functions
  ShapeFunctionSet nbvsfs(ordr);; //for  velocity

  //shape function size and total dof
  int vdof=vsfs.size()*dim; // two velocity components and total velocity sfs size

  //get parameter
  DGStokesParameters parameter;
  //get the geometry type of the face
  Dune::GeometryType gtface = isit.intersectionSelfLocal().type();
  //specify the quadrature order ?
  int qord=18;

  for (int qedg=0; qedg<Dune::QuadratureRules<ctype,dim-1>::rule(gtface,qord).size(); ++qedg)
  {
    //quadrature position on the edge/face in local=facelocal
    const Dune::FieldVector<ctype,dim-1>& local = Dune::QuadratureRules<ctype,dim-1>::rule(gtface,qord)[qedg].position();
    Dune:: FieldVector<ctype,dim> face_self_local = isit.intersectionSelfLocal().global(local);
    Dune:: FieldVector<ctype,dim> face_neighbor_local = isit.intersectionNeighborLocal().global(local);
    Dune::FieldVector<ctype,2> global = isit.intersectionGlobal().global(local);
    // calculating the inverse jacobian check if it is correct
    InverseJacobianMatrix inv_jac= ent.geometry().jacobianInverseTransposed(face_self_local);
    // get quadrature weight
    double quad_wt_face = Dune::QuadratureRules<ctype,dim-1>::rule(gtface,qord)[qedg].weight();
    ctype detjacface = isit.intersectionGlobal().integrationElement(local);
    // get the face normal-- unit normal.
    Dune::FieldVector<ctype,dim> normal = isit.unitOuterNormal(local);
    double norm_e= isit.intersectionGlobal().integrationElement(local);

    //================================================//
    // term to be evaluated : TERM:2
    //- \mu \int average(\nabla u). normal . jump(v)
    //================================================//
    // diagonal block
    // -mu* 0.5 * grad_phi_ei * normal* phi_ej
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,face_self_local);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          for (int sd=0; sd<2; sd++)
            temp[sd] = vsfs[j].evaluateDerivative(0,sd,face_self_local);
          grad_phi_ej[dm-1] = 0;
          // transform gradient to global ooordinates by multiplying with inverse jacobian
          inv_jac.umv(temp,grad_phi_ej[dm-1]);
          entry =-0.5 * parameter.mu * ((grad_phi_ej[dm-1]*normal)*phi_ei[dm-1])*detjacface*quad_wt_face;
          //Aee.add(ii,jj,entry);
          Aee[ii][jj]+=entry;
        }
      }
    }
    // offdiagonal entry
    // mu* 0.5 * grad_phi_ei * normal* phi_fj
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<nbvsfs.size(); ++i)
      {
        int ii=(dm-1)*nbvsfs.size()+i;
        phi_fi[dm-1] = nbvsfs[i].evaluateFunction(0,face_neighbor_local);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          for (int sd=0; sd<2; sd++)
            temp[sd] = vsfs[j].evaluateDerivative(0,sd,face_self_local);
          grad_phi_ej[dm-1] = 0;
          inv_jac.umv(temp,grad_phi_ej[dm-1]);
          entry =  0.5*parameter.mu*((grad_phi_ej[dm-1]*normal)*phi_fi[dm-1])*detjacface*quad_wt_face;
          //Afe.add(ii,jj,entry);
          Afe[ii][jj]+=entry;
        }
      }
    }
    //================================================//
    // term to be evaluated TERM:4
    // \mu \parameter.epsilon .\int average(\nabla v). normal . jump(u)
    //================================================//
    // diagonal term
    // mu* 0.5 * parameter.epsilon* phi_ei * grad_phi_ej* normal
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        for (int sd=0; sd<2; sd++)
          temp[sd] = vsfs[i].evaluateDerivative(0,sd,face_self_local);
        grad_phi_ei[dm-1] = 0;
        inv_jac.umv(temp,grad_phi_ei[dm-1]);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,face_self_local);
          entry=  0.5*parameter.mu*parameter.epsilon*(phi_ej[dm-1]*(grad_phi_ei[dm-1]*normal))*detjacface*quad_wt_face;
          //Aee.add(ii,jj,entry);
          Aee[ii][jj]+=entry;
        }
      }
    }
    // offdiagonal block
    // -mu* 0.5 * parameter.epsilon * grad_phi_ej * normal* phi_fi
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        for (int sd=0; sd<2; sd++)
          temp[sd] = vsfs[i].evaluateDerivative(0,sd,face_self_local);
        grad_phi_ei[dm-1] = 0;
        inv_jac.umv(temp,grad_phi_ei[dm-1]);
        // note test fns are now from neighbor element
        for (int j=0; j<nbvsfs.size(); ++j)
        {
          int jj=(dm-1)*nbvsfs.size()+j;
          phi_fj[dm-1] = nbvsfs[j].evaluateFunction(0,face_neighbor_local);
          entry =-0.5*parameter.mu*parameter.epsilon*( phi_fj[dm-1]*(grad_phi_ei[dm-1]*normal))*detjacface*quad_wt_face;
          Aef[ii][jj]+=entry;
        }
      }
    }

    //================================================//
    // term to be evaluated TERM:6
    //  term J0 =  \mu. (\parameter.sigma/norm(e)). jump(u). jump (v)
    //================================================//
    // Diagonalblock :
    // \mu. (\parameter.sigma/abs(e)).\int phi_ie. phi_je  where abs(e) =  norm (e) ???
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,face_self_local);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,face_self_local);
          entry=parameter.mu*(parameter.sigma/norm_e)*phi_ei[dm-1]*phi_ej[dm-1]*detjacface*quad_wt_face;
          Aee[ii][jj]+=entry;
        }
      }
    }
    // offdiagonal block
    //- mu*(parameter.sigma/norm_e)*phi_ei*phi_fj*detjacface*quad_wt_face;
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,face_self_local);
        int ii=(dm-1)*vsfs.size()+i;
        for (int j=0; j<nbvsfs.size(); ++j)                 // neighbor basis
        {
          int jj=(dm-1)*nbvsfs.size()+j;
          phi_fj[dm-1] = nbvsfs[j].evaluateFunction(0,face_neighbor_local);
          entry=-parameter.mu*(parameter.sigma/norm_e)*phi_ei[dm-1]*phi_fj[dm-1]*detjacface*quad_wt_face;
          Aef[ii][jj]+=entry;
        }
      }
    }
    //================================================//
    // term to be evaluated TERM:9
    //edge  term from B(v,p)
    // term \int average(p). jump(v).normal
    //================================================//
    //diagonal block
    // term==  0.5 * psi_ei. phi_ej* normal
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,face_self_local);
        for (int j=0; j<psfs.size(); ++j)
        {
          int jj=vdof+j;
          psi_ej = psfs[j].evaluateFunction(0,face_self_local);
          entry =0.5*(phi_ei[dm-1]*psi_ej*normal[dm-1])*detjacface*quad_wt_face;
          Aee[ii][jj]+=entry;
        }
      }
    }

    //offdiagonal block
    // term==  -0.5 * psi_ei. phi_fj* normal
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<nbvsfs.size(); ++i)
      {
        int ii=(dm-1)*nbvsfs.size()+i;
        phi_fi[dm-1] = nbvsfs[i].evaluateFunction(0,face_neighbor_local);
        for (int j=0; j<psfs.size(); ++j)                 // neighbor
        {
          int jj=vdof+j;
          psi_ej = psfs[j].evaluateFunction(0,face_self_local);
          entry = -0.5*(phi_fi[dm-1]*psi_ej*normal[dm-1])*detjacface*quad_wt_face;
          Afe[ii][jj]+=entry;
        }
      }
    }


    //================================================//
    // term to be evaluated TERM:12
    //edge  term from B(q,u)
    // term \int average(q). jump(u).normal
    // TERM:12
    //================================================//
    //diagonal block
    // term==  0.5 * psi_ej. phi_ei* normal
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<psfs.size(); ++i)
      {
        int ii=vdof+i;
        psi_ei = psfs[i].evaluateFunction(0,face_self_local);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,face_self_local);
          entry =0.5*(phi_ej[dm-1]*psi_ei*normal[dm-1])*detjacface*quad_wt_face;
          Aee[ii][jj]+=entry;
        }
      }
    }

    //offdiagonal block
    // term==  -0.5 * psi_ej. phi_fi* normal

    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<psfs.size(); ++i)
      {
        int ii=vdof+i;
        psi_ei = psfs[i].evaluateFunction(0,face_self_local);
        for (int j=0; j<nbvsfs.size(); ++j)                 // neighbor
        {
          phi_fj[dm-1] = nbvsfs[j].evaluateFunction(0,face_neighbor_local);
          int jj=(dm-1)*nbvsfs.size()+j;
          entry = -0.5*(phi_fj[dm-1]*psi_ei*normal[dm-1])*detjacface*quad_wt_face;
          Aef[ii][jj]+=entry;
        }
      }
    }

    //================================================//

  }      // end of assemble face quadrature loop

} // end of assemble face term


template<class G,int ordr>
void DGFiniteElementMethod<G,ordr>::assembleBoundaryTerm(Entity& ent, IntersectionIterator& isit, LocalMatrixBlock& Aee, LocalVectorBlock& Be) const
{
  Gradient grad_phi_ei[dim],grad_phi_ej[dim],temp;
  ctype phi_ei[dim],phi_ej[dim],psi_ei,psi_ej;
  ctype entry;
  ctype dirichlet[dim];
  //get the shape function set
  //self shape functions
  ShapeFunctionSet vsfs(ordr);; //for  velocity
  ShapeFunctionSet psfs(ordr-1); // for pressure
  //neighbor shape functions

  //shape function size and total dof
  int vdof=vsfs.size()*dim; // two velocity components and total velocity sfs size

  //get parameter
  DGStokesParameters parameter;
  //get the geometry type of the face
  Dune::GeometryType gtboundary = isit.intersectionSelfLocal().type();
  //specify the quadrature order ?
  int qord=18;
  for(int bq=0; bq<Dune::QuadratureRules<ctype,dim-1>::rule(gtboundary,qord).size(); ++bq)
  {
    const Dune::FieldVector<ctype,dim-1>& boundlocal = Dune::QuadratureRules<ctype,dim-1>::rule(gtboundary,qord)[bq].position();
    Dune:: FieldVector<ctype,dim> blocal = isit.intersectionSelfLocal().global(boundlocal);
    Dune::FieldVector<ctype,dim> bglobal = isit.intersectionGlobal().global(boundlocal);
    double norm_eb=isit.intersectionGlobal().integrationElement(boundlocal);
    // calculating the inverse jacobian
    InverseJacobianMatrix inv_jac= ent.geometry().jacobianInverseTransposed(blocal);
    // get quadrature weight
    double quad_wt_bound = Dune::QuadratureRules<ctype,dim-1>::rule(gtboundary,qord)[bq].weight();
    ctype detjacbound = isit.intersectionGlobal().integrationElement(boundlocal);
    // get the boundary normal
    Dune::FieldVector<ctype,dim> boundnormal = isit.unitOuterNormal(boundlocal);

    // finding velocity boundary condition
    //horizontal component
    exact_u(bglobal[0],bglobal[1],dirichlet[0]);
    //vertical component
    exact_v(bglobal[0],bglobal[1],dirichlet[1]);


    //================================================//
    //
    // TERM:3
    //- (\mu \int \nabla u. normal . v)
    //================================================//

    for(int dm=1; dm<=2; ++dm)
    {

      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,blocal);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          for (int sd=0; sd<2; sd++)
            temp[sd] = vsfs[j].evaluateDerivative(0,sd,blocal);
          grad_phi_ej[dm-1] = 0;
          inv_jac.umv(temp,grad_phi_ej[dm-1]);
          entry = ( - parameter.mu * ((grad_phi_ej[dm-1]*boundnormal)*phi_ei[dm-1])) * detjacbound*quad_wt_bound;
          Aee[ii][jj]+=entry;
        }
      }
    }
    //================================================//
    //TERM:5=  \mu parameter.epsilon \nabla v . normal. u
    //  TERM:15
    // rhs entry:  parameter.mu * parameter.epsilon* g * \nabla v * n
    //================================================//
    for(int dm=1; dm<=2; ++dm)
    {

      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        for (int sd=0; sd<2; sd++)
          temp[sd] = vsfs[i].evaluateDerivative(0,sd,blocal);
        grad_phi_ei[dm-1] = 0;
        inv_jac.umv(temp,grad_phi_ei[dm-1]);
        for (int j=0; j<vsfs.size(); ++j)
        {
          int jj=(dm-1)*vsfs.size()+j;
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,blocal);
          //TERM:5 \mu parameter.epsilon \nabla v . normal. u
          entry = parameter.mu *(parameter.epsilon*(grad_phi_ei[dm-1]*boundnormal)*phi_ej[dm-1] ) * detjacbound*quad_wt_bound;
          Aee[ii][jj]+=entry;
        }
        //------------------------------------
        //  TERM:15
        // rhs entry:  parameter.mu * parameter.epsilon* g * \nabla v * n
        //------------------------------------
        Be[ii]+= (parameter.epsilon*parameter.mu*(dirichlet[dm-1])*(grad_phi_ei[dm-1]*boundnormal)) * detjacbound * quad_wt_bound;
      }
    }

    //================================================//
    //  TERM:7
    // + \mu parameter.sigma/norm_e . v . u
    // TERM:16
    // rhs entry: mu*parameter.sigma/norm_e * g * v
    //================================================//
    for(int dm=1; dm<=2; ++dm)
    {

      for (int i=0; i<vsfs.size(); ++i)
      {

        phi_ei[dm-1] =  vsfs[i].evaluateFunction(0,blocal);
        int ii=(dm-1)*vsfs.size()+i;
        for (int j=0; j<vsfs.size(); ++j)
        {

          int jj=(dm-1)*vsfs.size()+j;
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,blocal);
          entry = ((parameter.mu*(parameter.sigma/norm_eb)*phi_ej[dm-1]*phi_ei[dm-1]))* detjacbound*quad_wt_bound;
          Aee[ii][jj]+=entry;
        }
        //------------------------------------
        // TERM:16
        // rhs entry: mu*parameter.sigma/norm_e * g * v
        //------------------------------------
        Be[ii]+= (parameter.mu*(parameter.sigma/norm_eb)*(dirichlet[dm-1])*phi_ei[dm-1])* detjacbound * quad_wt_bound;

      }

    }

    //================================================//
    // TERM:10
    //    \int p v n
    //================================================//
    for(int dm=1; dm<=2; ++dm)
    {
      for (int i=0; i<vsfs.size(); ++i)
      {
        int ii=(dm-1)*vsfs.size()+i;
        phi_ei[dm-1] = vsfs[i].evaluateFunction(0,blocal);
        for (int j=0; j<psfs.size(); ++j)
        {
          psi_ej = psfs[j].evaluateFunction(0,blocal);
          int jj=vdof+j;
          entry= (psi_ej*(phi_ei[dm-1]*boundnormal[dm-1]))* detjacbound * quad_wt_bound;
          Aee[ii][jj]+=entry;
        }
      }
    }

    //================================================//
    // \int q . u . n  --> TERM:13
    // psi_ej * phi_ei * normal
    //================================================//

    for(int dm=1; dm<=2; ++dm)
    {

      for (int i=0; i<psfs.size(); ++i)
      {
        int ii=vdof+i;
        psi_ei = psfs[i].evaluateFunction(0,blocal);
        for (int j=0; j<vsfs.size(); ++j)
        {
          phi_ej[dm-1] = vsfs[j].evaluateFunction(0,blocal);
          int jj=(dm-1)*vsfs.size()+j;
          entry= (psi_ei*(phi_ej[dm-1]*boundnormal[dm-1]) )* detjacbound * quad_wt_bound;
          Aee[ii][jj]+=entry;

        }
      }

    }

    //================================================//
    //TERM:17 (rhs)
    // \int q . g . n
    //================================================//
    for (int i=0; i<psfs.size(); ++i)
    {
      int ii=vdof+i;
      psi_ei = psfs[i].evaluateFunction(0,blocal);
      Be[ii]+=(dirichlet[0]*boundnormal[0]+dirichlet[1]*boundnormal[1])*psi_ei*detjacbound*quad_wt_bound;
    }
  }

}



template<class G,int ordr>
void DGStokes<G,ordr>::assembleStokesSystem()
{
  ShapeFunctionSet vsfs(ordr);; //for  velocity
  ShapeFunctionSet psfs(ordr-1); // for pressure

  int vdof=vsfs.size()*dim; // two velocity components and total velocity sfs size
  int pdof=psfs.size();
  int ndof=vdof+pdof; // total dofs per element
  int N = ndof*grid.size(level, 0);
  int nz=N;
  DGStokesParameters parameter;
  //sparserowmatrix
  Dune::SparseRowMatrix<double> SP(N,N,nz);
  AA=SP;
  Dune::SimpleVector<double> SV(N);
  bb=SV;
  bb=0.0;

  //istl matrix
  // N is the no of blocks of size=BlockSize
  // N is now no of elements
  N = grid.size(level, 0);
  Matrix tmp(N,N,Matrix::row_wise);
  typename Matrix::CreateIterator mit=tmp.createbegin();
  // build up the matrix structure
  ElementIterator eit = grid.template lbegin<0>(level);
  ElementIterator eitend = grid.template lend<0>(level);
  for (; eit != eitend; ++eit)
  {
    // insert a non zero entry for myself
    mit.insert(grid.levelIndexSet(level).index(*eit));
    assert(mit != tmp.createend());

    IntersectionIterator endit = eit->iend();
    IntersectionIterator iit = eit->ibegin();

    // insert a non zero entry for each neighbour
    for(; iit != endit; ++iit)
    {
      if (iit.neighbor())
      {
        mit.insert(grid.levelIndexSet(level).index(*iit.outside()));
      }
    }
    ++mit;
  }
  tmp = 0.0;
  A = tmp;
  Vector tmpv(N);
  b = tmpv;
  b = 0.0;


  // loop over all elements
  ElementIterator it = grid.template lbegin<0>(level);
  ElementIterator itend = grid.template lend<0>(level);
  for (; it != itend; ++it)
  {
    EntityPointer epointer = it;
    int eid = grid.levelIndexSet(level).index(*epointer);
    stokessystem.assembleVolumeTerm(*it,A[eid][eid],b[eid]);
    IntersectionIterator endis = it->iend();
    IntersectionIterator is = it->ibegin();
    for(; is != endis; ++is)
    {
      if(is.neighbor())
      {
        int fid = grid.levelIndexSet(level).index(*is.outside());
        stokessystem.assembleFaceTerm(*it,is,A[eid][eid],A[eid][fid],A[fid][eid],b[eid]);
      }
      if (is.boundary())
      {
        stokessystem.assembleBoundaryTerm(*it,is,A[eid][eid],b[eid]);
      }
    }
  }

  //changing istl matrix to spmatrix for superLU
  // supeLU needs matrix in supermatrix format
  for (typename Matrix::RowIterator i=A.begin(); i!=A.end(); ++i)
  {
    for (typename Matrix::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
    {
      for(int m=0; m<BlockSize; ++m)
      {
        for(int n=0; n<BlockSize; ++n)
        {
          AA.set(i.index()*BlockSize+m,j.index()*BlockSize+n,A[i.index()][j.index()][m][n]);
        }
      }
    }
  }
  // chainging block vector rhs to simple vector for superLU
  for(typename Vector::iterator i=b.begin(); i!=b.end(); ++i)
  {
    for(int m=0; m<BlockSize; ++m)
      bb[i.index()*BlockSize+m]=b[i.index()][m];
  }


  //modify matrix for introducing pressrure boundary condition
  // 12th row corresponds to pressure constant basis
  for(int j=0; j<N*ndof; ++j)
    AA.remove(12,j);
  AA.set(12,12,1);
  //rhs is set to zero
  bb[12]=0.0;

} // end of assemble



template<class G,int ordr>
void DGStokes<G,ordr>::solveStokesSystem()
{
  std::cout << "Solving Stokes System using superLU solver\n";

  //------------------superLU--------------------------//
  SuperMatrix _A;
  AA.createSuperMatrix(_A);
  SuperMatrix BB;
  dCreate_Dense_Matrix(&BB, bb.size(), 1, bb.raw(), bb.size(),
                       SLU_DN, SLU_D, SLU_GE);
  int n = bb.size();
  // setup the solver
  SuperMatrix L, U;
  int *perm_r = new int[n]; /* row permutations from partial pivoting */
  int *perm_c = new int[n]; /* column permutation vector */
  superlu_options_t options;

  SuperLUStat_t stat;
  int info;

  /* Set the default input options. */
  set_default_options(&options);
  //Initialize the statistics variables.
  StatInit(&stat);
  //solver
  dgssv(&options, &_A, perm_c, perm_r, &L, &U, &BB, &stat, &info);
  if ( options.PrintStat )
  {
    StatPrint ( &stat );
  }

  //#ifdef VERBOSE
  //dPrint_CompCol_Matrix ( "Matrix _A", &_A );
  //dPrint_SuperNode_Matrix ( "Factor L", &L );
  //dPrint_CompCol_Matrix ( "Factor U", &U );
  //dPrint_Dense_Matrix ( "Solution X", &BB );
  //#endif

  bb.print(1,"Solution:","row");

  // clean up superLU
  SUPERLU_FREE (perm_r);
  SUPERLU_FREE (perm_c);
  AA.destroySuperMatrix(_A);
  Destroy_SuperMatrix_Store(&BB);
  Destroy_SuperNode_Matrix(&L);
  Destroy_CompCol_Matrix(&U);
  StatFree(&stat);
  //------------------superLU--------------------------//

}

// template<class G,int ordr>
// double DGStokes<G,ordr>::l2errorStokesSystem() const
// {
//    long double error = 0.0;
//    // loop over all elements
//      ElementIterator it = grid.template lbegin<0>(level);
//     ElementIterator itend = grid.template lend<0>(level);
//      for (; it != itend; ++it)
//        {
//              Dune::FieldVector <ctype,dim> qp_loc(0.0);
//              Dune::FieldVector <ctype,dim> qp_glob(0.0);
//        }
// }


template <class G,int ordr>
double
DGFiniteElementMethod<G,ordr>::evaluateL2error(int component, Entity& element,const LocalVectorBlock& xe) const
{
  ExactSolution<double,dim> exact;
  double error[component]=0.0;
  Dune::FieldVector<ctype, dim> qp_loc(0.0);
  Dune::FieldVector<ctype, dim> qp_glob(0.0);
  Dune::GeometryType gt = element.geometry().type();
  //quadrature rule
  // need to find quad order based on the governing eqn..???
  int qord=18;
  for (int qp=0; qp<Dune::QuadratureRules<ctype,dim>::rule(gt,qord).size(); ++qp)
  {
    qp_loc = Dune::QuadratureRules<ctype,dim>::rule(gt,qord)[qp].position();
    qp_glob =element.geometry().global(qp_loc);
    double weight = Dune::QuadratureRules<ctype,dim>::rule(gt,qord)[qp].weight();
    double detjac = element.geometry().integrationElement(qp_loc);
    error[component]+=weight*detjac*exact.velocity(component,qp_glob);
  }
  return error[component];
}

// calculated value at local coord in e
template <class G, int ordr>
double
DGFiniteElementMethod<G,ordr>::evaluateSolution(int component,
                                                const Entity& element,
                                                const Dune::FieldVector< ctype, dim > & coord,
                                                const LocalVectorBlock & xe) const
{
  const ShapeFunctionSet & sfs = getShapeFunctionSet(element.geometry.type());
  int nsfs = sfs.size();
  ctype value = 0;
  for (int i=0; i<nsfs; ++i)
  {
    value += xe[i] * sfs[i].evaluateFunction(0, coord);
  }
  return value;
}

template <class G, int ordr>
inline const typename DGFiniteElementMethod<G,ordr>::ShapeFunctionSet &
DGFiniteElementMethod<G,ordr>::getShapeFunctionSet(Dune::GeometryType gt) const
{
  return space(gt, order);
}
