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

  /** \brief Vector class
   * This is the base class for all methods and operators.
   */
  template <typename Field>
  class Vector
  {
  public:
    //virtual Vector<Field> operator + (const Vector<Field> &) const = 0;
    //virtual Vector<Field> operator - (const Vector<Field> &) const = 0;
    //virtual Vector<Field> operator * (const Field &) const = 0;
    //virtual Vector<Field> operator / (const Field &) const = 0;

    /** \brief Assignment operator
        \note Only returns itself...
     */
    virtual Vector<Field>& operator  = (const Vector<Field> &) { return *this;};

    //! Addition
    virtual Vector<Field>& operator += (const Vector<Field> &) = 0;
    //! Subtraction
    virtual Vector<Field>& operator -= (const Vector<Field> &) = 0;
    //! Multiplication
    virtual Vector<Field>& operator *= (const Field &) = 0;
    //! Division
    virtual Vector<Field>& operator /= (const Field &) = 0;
  };


  /** @} end documentation group */

}


#endif
