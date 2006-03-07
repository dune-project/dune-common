// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef RHS_HH
#define RHS_HH


class RightHandSide
{
public:

  void u_rhs(const double&, const double&, double&);
  void v_rhs(const double&, const double&, double&);
  void p_rhs(const double&, const double&, double&);
};




void RightHandSide::u_rhs(const double& x, const double& y, double& f1)
{
  f1=sin(x)+y;
  //f1=-1;
  //f1=0;
  return ;
}
void RightHandSide::v_rhs(const double& x, const double& y, double& f2)
{
  f2=-y*cos(x)+x;
  //f2=0.0;
  return ;
}

void RightHandSide::p_rhs(const double& x, const double& y, double& f3)
{

  f3=0.0;
  return ;
}

#endif
