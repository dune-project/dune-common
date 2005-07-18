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
  public:
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

    /** \brief Send the matrix content to the screen*/
    void print() const {
      for (int row=0; row<rows_; row++) {
        for (int col=0; col<cols_; col++)
          std::cout << (*this)[row][col] << "  ";

        std::cout << std::endl;
      }
    }

    /** \brief Return the number of rows */
    int rows() const {
      return rows_;
    }

    /** \brief Return the number of columns */
    int cols() const {
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
      Matrix out(cols(), rows());
      for (int i=0; i<rows(); i++)
        for (int j=0; j<cols(); j++)
          out[j][i] = (*this)[i][j];

      return out;
    }

    //! Multiplication of the transposed matrix times a vector
    SimpleVector<T> transposedMult(const SimpleVector<T>& vec) {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (rows()!=vec.size())
        DUNE_THROW(ISTLError, "Vector size doesn't match the number of matrix rows!");
#endif
      SimpleVector<T> out(cols());
      out = 0;

      for (int i=0; i<out.size(); i++ ) {
        for ( int j=0; j<vec.size(); j++ )
          out[i] += (*this)[j][i]*vec[j];
      }

      return out;
    }

    /// Generic matrix multiplication.
    friend Matrix<T> operator*(const Matrix<T>& m1, const Matrix<T>& m2) {
      Matrix<T> out(m1.rows(), m2.cols());
      out.clear();

      for (int i=0; i<out.rows(); i++ ) {
        for ( int j=0; j<out.cols(); j++ )
          for (int k=0; k<m1.cols(); k++)
            out[i][j] += m1[i][k]*m2[k][j];
      }

      return out;
    }

    /// Generic matrix-vector multiplication.
    friend SimpleVector<T> operator*(const Matrix<T>& m, const SimpleVector<T>& vec) {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (cols()!=vec.size())
        DUNE_THROW(ISTLError, "Vector size doesn't match the number of matrix columns!");
#endif
      SimpleVector<T> out(m.rows());
      out = 0;

      for (int i=0; i<out.size(); i++ ) {
        for ( int j=0; j<vec.size(); j++ )
          out[i] += m[i][j]*vec[j];
      }

      return out;
    }

  protected:
    std::vector<T> data;

    int rows_;

    int cols_;
  };

} // end namespace Dune

#endif
