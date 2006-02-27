// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
class RHS
{
public:

  void u_rhs(const double&, const double&, double&);
  void v_rhs(const double&, const double&, double&);
  void p_rhs(const double&, const double&, double&);
};




void RHS::u_rhs(const double& x, const double& y, double& f1)
{
  f1=sin(x)+y;
  //f1=-1;
  //f1=0;
  return ;
}
void RHS::v_rhs(const double& x, const double& y, double& f2)
{
  f2=-y*cos(x)+x;
  //f2=0.0;
  return ;
}

void RHS::p_rhs(const double& x, const double& y, double& f3)
{

  f3=0.0;
  return ;
}



void exact_u(const double& x, const double& y, double& u)
{
  u= sin(x);
  //u=x*x;
  // u=y*(1.0-y);
  return ;
}
void exact_v(const double& x, const double& y, double& v)
{
  v=-y*cos(x);
  //v=-2*x*y;
  //v=0;
  return ;
}
void exact_p(const double& x, const double& y, double& p)
{
  p=x*y;
  //p=x;
  //p=0;
  return ;
}
