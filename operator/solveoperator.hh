// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SOLVEOPERATOR_HH__
#define __DUNE_SOLVEOPERATOR_HH__

#include "mapping.hh"

namespace Dune
{

  template<typename Field, class Domain, class Range>
  class SolveOperator : Operator < Field, Domain, Range > {

  public:
    virtual Vector<Field> operator + (const Vector<Field> &) const ;
    virtual Vector<Field> operator - (const Vector<Field> &) const ;
    virtual Vector<Field> operator * (const Field &) const  ;
    virtual Vector<Field> operator / (const Field &) const  ;
    virtual Vector<Field>& operator  = (const Vector<Field> &) ;
    virtual Vector<Field>& operator += (const Vector<Field> &) ;
    virtual Vector<Field>& operator -= (const Vector<Field> &) ;
    virtual Vector<Field>& operator *= (const Field &)  ;
    virtual Vector<Field>& operator /= (const Field &)  ;


    virtual void operator () ( const Domain & , Range &) const ;


  private:


  };

}

#endif
