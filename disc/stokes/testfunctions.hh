// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef TESTFUNCTIONS_HH
#define TESTFUNCTIONS_HH


template<class ct,int dim>
class ExactSolution
{
public:
  typedef Dune::FieldVector<ct,dim> Point;

  //ExactSolution(){}

  virtual ct velocity(int comp,const Point & global) const
  {
    return 0;
  }
  virtual ct pressure(const Point & global) const
  {
    return 0;
  }
  //virtual ~ExactSolution() {}
};


// void exact_u(const double& x, const double& y, double& u)
// {
//   u= sin(x);
//   //u=x*x;
//   // u=y*(1.0-y);
//    return ;
// }
// void exact_v(const double& x, const double& y, double& v)
// {
//   v=-y*cos(x);
//   //v=-2*x*y;
//   //v=0;
//    return ;
// }
// void exact_p(const double& x, const double& y, double& p)
// {
//   p=x*y;
//   //p=x;
//   //p=0;
//    return ;
// }

#include "testfunctions.cc"
#endif
