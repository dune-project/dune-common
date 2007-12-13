// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ASSIGN_HH
#define DUNE_ASSIGN_HH

#include <dune/common/fvector.hh>

namespace Dune {

  /**
   * Emtpy namespace make this class and the object local to one object file
   */
  namespace {
    /**
     *  @brief Initializer class for
     *
     *  overload operator <<= for fvector assignment from Dune::Zero
     */
    class Zero {
      /** \brief Conversion operator to double */
      operator double () { return 0.0; }
      /** \brief Conversion operator to int */
      operator int () { return 0; }
    } zero;
  } // end empty namespace

  /**
   *  @brief fvector assignment operator
   *
   *  overload operator <<= for fvector assignment from Dune::Zero
   *
   *  after including fassing.hh you can easily assign data to a FieldVector
   *  using
   *
   *  @code
   *  FieldVector<double, 4> x; x <<= 1.0, 4.0, 10.0, 11.0;
   *  @endcode
   *
   *  The operator checks that the whole vector is initalized.
   *  In case you know that all following entries will be zero padded, you can use
   *
   *  @code
   *  FieldVector<double, 40> x; x <<= 1.0, 4.0, 10.0, 11.0, zero;
   *  @endcode
   *
   */
  template <class T, int s>
  class fvector_assigner
  {
  private:
    FieldVector<T,s> & v;
    int c;
    bool temporary;
  public:
    /*! @brief Copy Constructor */
    fvector_assigner(fvector_assigner & a) : v(a.v), c(a.c), temporary(false)
    {}
    /*! @brief Constructor from vector and temporary flag
       \param v vector which should be initialized
       \param t bool indicating, that this is a temporary object (see ~fvector_assigner)
     */
    fvector_assigner(FieldVector<T,s> & _v, bool t) : v(_v), c(0), temporary(t)
    {};
    /*! @brief Destructor
       checks for complete initialization of the vector.
       The check is skipped, if this object is marked temporary.
     */
    ~fvector_assigner()
    {
      if (!temporary && c!=s)
        DUNE_THROW(MathError, "Trying to assign " << c <<
                   " entries to a FieldVector of size " << s);
    }
    /*! @brief append data to this vector */
    fvector_assigner & append (const T & t)
    {
      v[c++] = t;
      return *this;
    }
    /*! @brief append zeros to this vector
     */
    fvector_assigner & append (Zero z)
    {
      while (c!=s) v[c++] = 0;
      return *this;
    }
    /*! @brief append data to this vector
       the overloaded comma operator is used to assign a comma seperated list
       of values to the vector
     */
    fvector_assigner & operator , (const T & t)
    {
      return append(t);
    }
    /*! @brief append zeros to this vector
       the overloaded comma operator is used to stop the assign of values
       to the vector, all remaining entries are assigned 0.
     */
    fvector_assigner & operator , (Zero z)
    {
      return append(z);
    }
  };

  /**
   *  @brief fvector assignment operator
   *
   *  overload operator <<= for fvector assignment
   *  from comma seperated list of values
   */
  template <class T, int s>
  fvector_assigner<T,s> operator <<= (FieldVector<T,s> & v, const T & t)
  {
    return fvector_assigner<T,s>(v,true).append(t);
  }

  /**
   *  @brief fvector assignment operator
   *
   *  overload operator <<= for fvector assignment from Dune::Zero
   */
  template <class T, int s>
  fvector_assigner<T,s> operator <<= (FieldVector<T,s> & v, Zero z)
  {
    return fvector_assigner<T,s>(v,true).append(z);
  }

} // end namespace Dune

#endif // DUNE_ASSIGN_HH
