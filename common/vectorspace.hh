// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_VECTORSPACE_HH__
#define __DUNE_VECTORSPACE_HH__

namespace Dune {


  template <typename Field> class Vector
  {
  public:
    virtual Vector<Field> operator + (const Vector<Field> &) const { return (*this); };
    virtual Vector<Field> operator - (const Vector<Field> &) const { return (*this); };
    virtual Vector<Field> operator * (const Field &) const { return (*this); };
    virtual Vector<Field> operator / (const Field &) const { return (*this); };
    virtual Vector<Field>& operator  = (const Vector<Field> &) { return (*this); };
    virtual Vector<Field>& operator += (const Vector<Field> &){ return (*this); };
    virtual Vector<Field>& operator -= (const Vector<Field> &){ return (*this); };
    virtual Vector<Field>& operator *= (const Field &){ return (*this); };
    virtual Vector<Field>& operator /= (const Field &){ return (*this); };
  };


}


#endif
