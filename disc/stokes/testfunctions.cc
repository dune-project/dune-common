// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
# include "config.h"     // autoconf defines, needed by the dune headers
#endif
#include "testfunctions.hh"

// here are some sample problem with known solution

template<int dim, class ct>
class Example : public ExactSolution<ct, dim>
{
  typedef Dune::FieldVector< ct, dim > Point;

public:
  Example(){}

  ct velocity(int comp,const Point & glob) const
  {
    if (comp==0) return sin(glob[0]);   // sin(x)
    if (comp==1) return -glob[1]*cos(glob[0]);      // -y*cos(x)
  }
  ct pressure(const Point & glob) const
  {
    return glob[0]*glob[1];     // x*y

  }
  ct rhsvalue(int variable, const Point& glob) const
  {
    // stokes system has dim+1 variables (dim velocity comps and 1 pressure)
    if (variable==0) return sin(glob[0])+glob[1];     // sin(x)+y
    if (variable==1) return -glob[1]*cos(glob[0])+glob[0];    // -y*cos(x)+x
    if (variable==2) return 0.0;
  }
  virtual ~Example(){}
};

template<int dim, class ct>
class PoiseuilleFlow : public ExactSolution<ct, dim>
{
  typedef Dune::FieldVector< ct, dim > Point;

public:
  PoiseuilleFlow(){}

  ct velocity(int comp,const Point & glob) const
  {
    if (comp==0) return glob[1]*(1-glob[1]);   // y*(1-y)
    if (comp==1) return 0;       // 0
  }
  ct pressure(const Point & glob) const
  {
    return 0;

  }
  ct rhsvalue(int variable, const Point& glob) const
  {
    // stokes system has dim+1 variables (dim velocity comps and 1 pressure)
    if (variable==0) return 0;
    if (variable==1) return 0;
    if (variable==2) return 0;
  }
  virtual ~PoiseuilleFlow(){}
};
