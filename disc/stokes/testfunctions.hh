// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef TESTFUNCTIONS_HH
#define TESTFUNCTIONS_HH


template<class ct,int dim>
class ExactSolution
{
public:
  typedef Dune::FieldVector<ct,dim> Point;

  ExactSolution(){}

  virtual ct velocity(int variable,const Point & global) const = 0;

  virtual ct pressure(const Point & global) const = 0;

  virtual ct rhsvalue(int variable, const Point& global) const =0;

  virtual ~ExactSolution() {}
};

#include "testfunctions.cc"
#endif
