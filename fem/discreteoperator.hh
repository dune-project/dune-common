// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCRETEOPERATOR_HH
#define DUNE_DISCRETEOPERATOR_HH

#include "discretefunction.hh"
#include "../grid/sgrid.hh"

#include "operator/spmatrix.hh"
#include "operator/CG.hh"


namespace Dune
{


  //! general definition on an operator
  class Operator
  {
  public:
    // apply to function
    template <class Func>
    Func& operator () (Func &f);
  }; // end Operator


  class DiscreteOperator : public Operator
  {
  public:
    DiscreteOperator() {};

    // preStep, create matrix or something
    template <class DiscFunc>
    void assemble (DiscFunc &func);

    // apply
    template <class DiscFunc>
    DiscFunc& operator () (DiscFunc &f);

    // postStep, do after apply
    template <class DiscFunc>
    void finalize (DiscFunc &f);

  }; //end DiscreteOperator

#if 0
  //***************************************************************************
  //
  //  --FV1stOrd Finite Volume scheme first order
  //  implements a special DiscretOperator
  //
  //***************************************************************************
  class FV1stOrd : public DiscreteOperator
  {
    ScalarVector *helpVec_;
    double dt_;
    bool built;
  public:
    FV1stOrd();
    ~FV1stOrd();

    // preStep
    template <class DiscFunc>
    void assemble (DiscFunc &func, double dt);

    // solve
    template <class DiscFunc>
    DiscFunc& operator () (DiscFunc &f);

    // postStep
    template <class DiscFunc>
    void finalize (DiscFunc &f);

  private:
    template <class DiscFunc>
    void solve(DiscFunc &f);

  }; //end FV1stOrd
#endif


  //***************************************************************************
  //
  //  --LinFEM Linear Finite Elements
  //  implements a special DiscretOperator
  //
  //***************************************************************************
  class LinFEM : public DiscreteOperator
  {
    enum { maxcol_ = 15 };

    ScalarVector *helpVec_;

    SparseRowMatrix<double>* matrix_;

    int myDim_;
    double dt_;
    bool built;
  public:
    LinFEM();
    ~LinFEM();

    // preStep
    template <class DiscFunc>
    void assemble (DiscFunc &func, double dt, double time);

    // solve
    template <class DiscFunc>
    DiscFunc& operator () (DiscFunc &f);

    // postStep
    template <class DiscFunc>
    void finalize (DiscFunc &f);

  private:
    template <class DiscFunc, class Entity>
    void boundaryValues(DiscFunc &f, Entity &el);

    template <class DiscFunc>
    void solve(DiscFunc &f);
    void remove();

    template <class DiscFunc>
    void assembleMatrix(DiscFunc &func, int level);

  }; //end FV1stOrd



  //*************************************************************
  //
  // --TimeEulerFV
  //
  //*************************************************************
  template <class FVdiscr>
  class TimeEulerFV : public DiscreteOperator
  {
    typedef SGrid<1,1> TimeGrid;

    FVdiscr *fv_;
    bool built;
    TimeGrid *timegrid_;

  public:
    TimeEulerFV(); // Startwert
    ~TimeEulerFV();

    // preStep
    template <class DiscFunc>
    void assemble (DiscFunc &f, double startTime, double endTime);

    // solve
    template <class DiscFunc>
    DiscFunc& operator () (DiscFunc &f);

    // postStep
    template <class DiscFunc>
    void finalize (DiscFunc &f);

    template <class DiscFunc>
    DiscFunc& operator += (DiscFunc &f);

  }; //end TimeEuler

} // end namespace Dune

#include "operator/discreteoperator.cc"

#endif
