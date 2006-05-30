// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MATRIX_HH
#define DUNE_MATRIX_HH

#include <vector>
#include "simplevector.hh"

namespace Dune {

  /** \brief A generic dynamic matrix
      \ingroup Common
   */
  template<class T>
  class Matrix
  {
    class ISTLError : public Exception {};
  public:

    /** \brief Export the type representing the components */
    typedef T block_type;

    /** \brief Type for indices and sizes */
    typedef int size_type;

    /** \brief Create empty matrix */
    Matrix() : data(0), rows_(0), cols_(0)
    {}

    /** \brief Create uninitialized matrix of size rows x cols
     */
    Matrix(int rows, int cols) : data(rows*cols), rows_(rows), cols_(cols)
    {}

    /** \brief Change the matrix size
     *
     * The way the data is handled is unpredictable.
     */
    void resize(int rows, int cols) {
      data.resize(rows*cols);
      rows_ = rows;
      cols_ = cols;
    }

    /** \brief Write a zero into all matrix entries
     */
    void clear() {
      for (unsigned int i=0; i<data.size(); i++)
        data[i] = 0;
    }

    /** \brief Assignment from scalar */
    Matrix& operator= (const T& t)
    {
      for (unsigned int i=0; i<data.size(); i++)
        data[i] = t;
    }

    /** \brief The index operator */
    T* operator[](int row) {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (row<0)
        DUNE_THROW(ISTLError, "Can't access negative rows!");
      if (row>=rows_)
        DUNE_THROW(ISTLError, "Row index out of range!");
#endif
      return &data[row*cols_];
    }

    /** \brief The const index operator */
    const T* operator[](int row) const {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (row<0)
        DUNE_THROW(ISTLError, "Can't access negative rows!");
      if (row>=rows_)
        DUNE_THROW(ISTLError, "Row index out of range!");
#endif
      return &data[row*cols_];
    }

    /** \brief Return the number of rows */
    size_type N() const {
      return rows_;
    }

    /** \brief Return the number of columns */
    size_type M() const {
      return cols_;
    }

    /** \brief Multiplication with a scalar */
    Matrix<T> operator*=(const T& scalar) {
      for (int row=0; row<rows_; row++)
        for (int col=0; col<cols_; col++)
          (*this)[row][col] *= scalar;

      return (*this);
    }

    /** \brief Return the transpose of the matrix */
    Matrix transpose() const {
      Matrix out(N(), M());
      for (int i=0; i<M(); i++)
        for (int j=0; j<N(); j++)
          out[j][i] = (*this)[i][j];

      return out;
    }

    //! Multiplication of the transposed matrix times a vector
    SimpleVector<T> transposedMult(const SimpleVector<T>& vec) {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (N()!=vec.size())
        DUNE_THROW(ISTLError, "Vector size doesn't match the number of matrix rows!");
#endif
      SimpleVector<T> out(M());
      out = 0;

      for (int i=0; i<out.size(); i++ ) {
        for ( int j=0; j<vec.size(); j++ )
          out[i] += (*this)[j][i]*vec[j];
      }

      return out;
    }

    /// Generic matrix multiplication.
    friend Matrix<T> operator*(const Matrix<T>& m1, const Matrix<T>& m2) {
      Matrix<T> out(m1.N(), m2.M());
      out.clear();

      for (int i=0; i<out.N(); i++ ) {
        for ( int j=0; j<out.M(); j++ )
          for (int k=0; k<m1.M(); k++)
            out[i][j] += m1[i][k]*m2[k][j];
      }

      return out;
    }

    /// Generic matrix-vector multiplication.
    friend SimpleVector<T> operator*(const Matrix<T>& m, const SimpleVector<T>& vec) {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (M()!=vec.size())
        DUNE_THROW(ISTLError, "Vector size doesn't match the number of matrix columns!");
#endif
      SimpleVector<T> out(m.N());
      out = 0;

      for (int i=0; i<out.size(); i++ ) {
        for ( int j=0; j<vec.size(); j++ )
          out[i] += m[i][j]*vec[j];
      }

      return out;
    }

    //! y += A x
    template <class X, class Y>
    void umv(const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<rows_; i++) {

        for (int j=0; j<cols_; j++)
          (*this)[i][j].umv(x[j], y[i]);

      }

    }

  protected:
    std::vector<T> data;

    int rows_;

    int cols_;
  };

  //! Send Matrix to an output stream
  template<class T>
  std::ostream& operator<< (std::ostream& s, const Matrix<T>& m)
  {
    for (int row=0; row<m.N(); row++) {
      for (int col=0; col<m.M(); col++)
        s << m[row][col] << "  ";

      s << std::endl;
    }

    return s;
  }

} // end namespace Dune

#endif
