// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_VECTORSPACE_HH__
#define __DUNE_VECTORSPACE_HH__

namespace Dune {

  /** @defgroup AlgebraCommon Functions and Operators
      The Functions and Operators Module contains objects of
      arbitrary vector spaces and mappings between them.
     @{
   */

  /** @defgroup Vector Elements of Vector Spaces
      An instance of this class is an element of an vector space.
      Elements of vector spaces can be added and multiplied by a
      scalar.
     @{
   */
  template <typename Field> class Vector
  {
  public:
    //virtual Vector<Field> operator + (const Vector<Field> &) const = 0;
    //virtual Vector<Field> operator - (const Vector<Field> &) const = 0;
    //virtual Vector<Field> operator * (const Field &) const = 0;
    //virtual Vector<Field> operator / (const Field &) const = 0;

    //! note: Oli changed all method to, I changed it back, because
    //! otherwise the code will not compile and I fixed this hack later.
    virtual Vector<Field>& operator  = (const Vector<Field> &) { return *this;};

    //! Methods are pure virtual, that the user is forced to implement them
    virtual Vector<Field>& operator += (const Vector<Field> &) = 0;
    virtual Vector<Field>& operator -= (const Vector<Field> &) = 0;
    virtual Vector<Field>& operator *= (const Field &) = 0;
    virtual Vector<Field>& operator /= (const Field &) = 0;
  };


  /** @} end documentation group */

}


#endif
