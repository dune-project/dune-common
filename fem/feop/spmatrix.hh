// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SPMATRIX_HH
#define __DUNE_SPMATRIX_HH

#include <dune/common/simplevector.hh>

namespace Dune
{

  //*****************************************************************
  //
  //  --SparseRowMatrix
  //
  //! Compressed row sparse matrix, where only the nonzeros of a row are
  //! kept.
  //*****************************************************************
  template <class T>
  class SparseRowMatrix
  {
  public:
    typedef T Ttype; //!< remember the value type

  private:
    Array<T> values_;     //!< data values (nz_ elements)
  public:
    Array<int> col_;      //!< row_ptr (dim_[0]+1 elements)
  private:
    int dim_[2];    //!< dim_[0] x dim_[1] Matrix
    int nz_;        //!< number of nonzeros per row

  public:

    //! makes Matrix of zero length
    SparseRowMatrix();

    //! Copy Constructor
    SparseRowMatrix(const SparseRowMatrix<T> &S);

    //! make matrix with 'rows' rows and 'cols' columns,
    //! maximum 'nz' non zero values in each row
    //! and initialize all values with zero
    SparseRowMatrix(int rows, int cols, int nz);

    //! free memory for values_ and col_
    ~SparseRowMatrix();

    //! Resize matrix
    void resize(int rows, int cols, int nz);

    //! Resize matrix keeping number of nonzero entries per row fixed
    void resize(int rows, int cols);

    //! Set all matrix entries to zero
    void clear();

    //! Deep copy operator
    SparseRowMatrix<T>& SparseRowMatrix<T>::operator=(const SparseRowMatrix<T>& other);

    /*******************************/
    /*  Access and info functions  */
    /*******************************/

    //! Direct access to the internal data array
    T&      val(int i) { return values_[i]; }

    //! Direct const access to the internal data array
    const T&  val(int i) const { return values_[i]; }

    /** \brief Gets the position of an entry in the internal data structure
     *
     * \return <ul>
     * <li> -1: if the entry couldn't be found and the data table is full </li>
     * <li> else: the column number of either the entry or a free slot in the table </li>
     * </ul>
     */
    int colIndex(int row, int col);

    //! Returns number of matrix rows (i==0) and columns (i==1)
    int size(int i) const {return dim_[i];}

    //! Returns the number of columns
    int rows() const {return size(0);}

    //! Returns the number of columns
    int cols() const {return size(1);}

    //! Returns total number of nonzero entries
    int numNonZeros() const {return nz_;}

    //! Const index operator
    const T&  operator() (int i, int j) const;

    //! Set matrix entry
    void set(int row, int col, const T& val);

    //! Add to matrix entry
    void add(int row, int col, const T& val);

    //! Multiply an entry with a scalar
    void multScalar(int row, int col, const T& val);

    //! Multiply the whole matrix with a scalar
    const SparseRowMatrix<T>& operator*=(const T& val);

    //! Makes 'row' a unit row and 'col' a unit column
    void kroneckerKill(int row, int col);

    /** \brief Multiply with a vector
     *
     * This method works on arrays and expects the template
     * parameter type to have an index operator.
     */
    template <class VECtype>
    void mult(VECtype &ret, const VECtype& x) const;

    //! Multiply with a SimpleVector
    SimpleVector<T> operator*(const SimpleVector<T>& v) const;

    /** \brief Multiply with a vector
     *
     * This method works on discrete functions and expects
     * the template parameter types to have DofIterators.
     */
    template <class DiscFType, class DiscFuncType>
    void apply(const DiscFType &f, DiscFuncType &ret) const;

    //! Multiply the transpose with a vector
    template <class DiscFuncType>
    void apply_t(const DiscFuncType &f, DiscFuncType &ret) const;

    /** \brief Multiply the transpose with a vector
     */
    void apply_t(const SimpleVector<T> &f, SimpleVector<T> &ret) const;

    /** \brief For an argument \f$ A \f$, this computes \f$ M A M^T \f$
     * \todo This routine's complexity still scales linearly with the total number
     * of matrix entries of the result (not only the nonzero ones).
     * Isn't there anything better?
     */
    SparseRowMatrix<T> applyFromLeftAndRightTo(const SparseRowMatrix<T>& A) const;

    //! Prints the complete matrix including the nonzero entries
    void print (std::ostream& s) const;

    //! Just prints the nonzero entries
    void printReal (std::ostream& s) const;

    //! Makes a given row a unit row
    void unitRow(int row);

    //! Makes a given column a unit column
    void unitCol(int col);
  private:

    //! Always contains zero.  It's only here so the index operator
    //! can return a const reference to a zero entry with triggering a compiler warning.
    static T staticZero;

  };


} // end namespace Sparselib

#include "spmatrix.cc"

#endif
