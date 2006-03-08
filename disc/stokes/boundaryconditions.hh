// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef BOUNDARYCONDITIONS_HH
#define BOUNDARYCONDITIONS_HH

#include "testfunctions.hh"


template<class Grid>
class DirichletBoundary
{

  enum {dim=Grid::dimension};
  typedef typename Grid::ctype ct;
public:
  DirichletBoundary(ExactSolution<ct,dim>& ex) : exact(ex){}
  typedef Dune::FieldVector<ct,dim> Point;
  double dirichletValue(int comp,const Point& global, Point& local) const;
protected:
  ExactSolution<ct,dim>& exact;
};

#include "boundaryconditions.cc"
#endif

// template<class Grid>
// class DirichletBoundary
// {

//   enum{dim=Grid::dimension};
//   typedef typename Grid::ctype ct;
// public:
//   typedef Dune::FieldVector<ct,dim> Point;

//    virtual double dirichletValue(int comp,const Point& global, ct local) const;
//  protected:
//    ExactSolution<ct,dim>& exact;
// };
