// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ASSIGN_HH
#define DUNE_ASSIGN_HH

#warning The header <dune/common/fassign.hh> is deprecated and will be removed after the\
         release of dune-common-2.4. Please use C++11 initializer lists instead.

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/unused.hh>

namespace Dune {

  /**
   * @file
   * @brief Classes for implementing an assignment to FieldVector from a comma-separated list
   */
  /** @addtogroup Common
   *
   * @{
   */

  /**
   * Emtpy namespace make this class and the object local to one object file
   */
  namespace {

    /**
     *  @brief Initializer class for
     *
     *  overload operator <<= for FieldVector assignment from Dune::Zero
     */
    struct Zero {
      explicit Zero (int) {};
      /** \brief Conversion operator to double */
      operator double () { return 0.0; }
      /** \brief Conversion operator to int */
      operator int () { return 0; }
    } zero(0);

    /**
     *  @brief Marker class for next row
     *
     *  overload operator <<= for FieldMatrix assignment
     */
    struct NextRow {
      explicit NextRow (int) {};
    } nextRow(0);
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
    fvector_assigner();
  public:
    /*! @brief Copy Constructor */
    fvector_assigner(fvector_assigner & a) : v(a.v), c(a.c), temporary(false)
    {}
    /*! @brief Constructor from vector and temporary flag
       \param _v vector which should be initialized
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
      DUNE_UNUSED_PARAMETER(z);
      while (c!=s) v[c++] = 0;
      return *this;
    }
    /*! @brief append data to this vector
       the overloaded comma operator is used to assign a comma separated list
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
   *  from comma separated list of values
   */
  template <class T, class K, int s>
  fvector_assigner<T,s> operator <<= (FieldVector<T,s> & v, const K & t)
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

  /**
   *  @brief fmatrix assignment operator
   *
   *  overload operator <<= for fmatrix assignment from Dune::Zero
   *
   *  after including fassing.hh you can easily assign data to a FieldMatrix
   *  using
   *
   *  @code
   *  FieldMatrix<double, 2,2> x; x <<= 1.0, 4.0, nextRow, 10.0, 11.0;
   *  @endcode
   *
   *  The operator checks that the whole matrix is initalized.
   *  In case you know that all following entries of a row will be zero padded, you can use
   *
   *  @code
   *  FieldMatrix<double, 4, 4> x; x <<= 1.0, zero, nextRow, 10.0, 11.0;
   *  @endcode
   *
   */
  template <class T, int n, int m>
  class fmatrix_assigner
  {
  private:
    FieldMatrix<T,n,m> & A;
    int c;
    int r;
    bool temporary;
    bool thrown;

    void end_row()
    {
      if (!temporary && c!=m && !thrown) {
        thrown=true;
        DUNE_THROW(MathError, "Trying to assign " << c <<
                   " entries to a FieldMatrix row of size " << m);
      }
      c=0;
    }
  public:
    /*! @brief Copy Constructor */
    fmatrix_assigner(fmatrix_assigner & a) : A(a.A), c(a.c), r(a.r), temporary(false), thrown(a.thrown)
    {}
    /*! @brief Constructor from matrix and temporary flag
       \param _A matrix which should be initialized
       \param t bool indicating, that this is a temporary object (see ~fmatrix_assigner)
     */
    fmatrix_assigner(FieldMatrix<T,n,m> & _A, bool t) : A(_A), c(0), r(0), temporary(t),
                                                        thrown(false)
    {};
    /*! @brief Destructor
       checks for complete initialization of the matrix.
       The check is skipped, if this object is marked temporary.
     */
    ~fmatrix_assigner()
    {
      end_row();
      if (!temporary && r!=n-1 && !thrown) {
        thrown=true;
        DUNE_THROW(MathError, "Trying to assign " << r <<
                   " rows to a FieldMatrix of size " << n << " x " << m);
      }
    }
    /*! @brief append data to this matrix */
    fmatrix_assigner & append (const T & t)
    {
      // Check whether we have passed the last row
      if(r>=m) {
        thrown=true;
        DUNE_THROW(MathError, "Trying to assign more than " << m <<
                   " rows to a FieldMatrix of size " << n << " x " << m);
      }
      A[r][c++] = t;
      return *this;
    }
    /*! @brief append zeros to this matrix
     */
    fmatrix_assigner & append (Zero z)
    {
      DUNE_UNUSED_PARAMETER(z);
      while (c!=m) A[r][c++] = 0;
      return *this;
    }
    /*! @brief move to next row of the matrix
     */
    fmatrix_assigner & append (NextRow nr)
    {
      DUNE_UNUSED_PARAMETER(nr);
      end_row();
      r++;
      return *this;
    }
    /*! @brief append data to this matrix
       the overloaded comma operator is used to assign a comma separated list
       of values to the matrix
     */
    fmatrix_assigner & operator , (const T & t)
    {
      return append(t);
    }
    /*! @brief append zeros to this matrix
       the overloaded comma operator is used to stop the assign of values
       to the matrix, all remaining entries are assigned 0.
     */
    fmatrix_assigner & operator , (Zero z)
    {
      return append(z);
    }
    /*! @brief append zeros to this matrix
       the overloaded comma operator is used to stop the assign of values
       to the current row, it will be checked whether all entries have been
       assigned values.
     */
    fmatrix_assigner & operator , (NextRow nr)
    {
      return append(nr);
    }
  };

  /**
   *  @brief FieldMatrix assignment operator
   *
   *  overload operator <<= for FieldMatrix assignment
   *  from comma separated list of values
   */
  template <class T, class K, int n, int m>
  fmatrix_assigner<T,n,m> operator <<= (FieldMatrix<T,n,m> & v, const K & t)
  {
    return fmatrix_assigner<T,n,m>(v,true).append(t);
  }

  /**
   *  @brief FieldMatrix assignment operator
   *
   *  overload operator <<= for FieldMatrix row assignment from Dune::Zero
   */
  template <class T, int n, int m>
  fmatrix_assigner<T,n,m> operator <<= (FieldMatrix<T,n,m> & v, Zero z)
  {
    return fmatrix_assigner<T,n,m>(v,true).append(z);
  }

} // end namespace Dune

#endif // DUNE_ASSIGN_HH
