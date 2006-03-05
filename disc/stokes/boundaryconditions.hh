// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef BOUNDARYCONDITIONS_HH
#define BOUNDARYCONDITIONS_HH



template<class ct,int dim>
class ExactSolution
{
public: typedef Dune::FieldVector<ct,dim> Point;
  virtual ~ExactSolution(){};
  virtual ct velocity(int comp,const Point & global) const =0;
  virtual ct pressure(const Point & global) const =0;

};

template<class Grid>
class DirichletBoundary
{

  enum {dim=Grid::dimension};
  typedef typename Grid::ctype ct;
public:
  typedef Dune::FieldVector<ct,dim> Point;

  virtual double dirichletValue(int comp,const Point& global, ct local) const =0;
protected:
  ExactSolution<ct,dim>& exact;
};

#include "boundaryconditions.cc"
#endif
