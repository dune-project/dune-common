// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SPMATRIX_HH
#define __DUNE_SPMATRIX_HH

namespace Dune
{

  //*****************************************************************
  //
  //  --SparseRowMatrix
  //
  //! Compressed row sparse matrix, where only the nonzeros of a row a
  //! keeped
  //*****************************************************************
  template <class T>
  class SparseRowMatrix
  {
  public:
    typedef T Ttype; //!< remember the value type

  private:
    T* values_;     //! data values (nz_ elements)
    int* col_;      //! row_ptr (dim_[0]+1 elements)
    int dim_[2];    //! dim_[0] x dim_[1] Matrix
    int nz_;        //! number of nonzeros per row

  public:

    //! makes Matrix of zero length
    SparseRowMatrix();

    //! Copy Constructor
    SparseRowMatrix(const SparseRowMatrix<T> &S);

    //! make matrix with 'rows' rows and 'cols' columns,
    //! maximum 'nz' non zero values in each row
    //! and intialize all values with 'val'
    SparseRowMatrix(int rows, int cols, int nz, T val);

    //! free memory for values_ and col_
    ~SparseRowMatrix();

    /*******************************/
    /*  Access and info functions  */
    /*******************************/

    //! ???
    T&      val(int i) { return values_[i]; }

    //! ???
    const T&  val(int i) const { return values_[i]; }

    //! ???
    int colIndex(int row, int col);

    //const int&         row_ptr(int i) const { return rowptr_(i); }
    //const int&         col_ind(int i) const { return colind_(i);}

    //! Returns number of matrix rows (i==0) and columns (i==1)
    int dim(int i) const {return dim_[i];};

    //! Returns number of matrix rows (i==0) and columns (i==1)
    int size(int i) const {return dim_[i];};

    //! Returns total number of nonzero entries
    int numNonZeros() const {return nz_;};

    //int base() const {return base_;}

    //! Const index operator
    const T&  operator() (int i, int j) const;

    //! Set matrix entry
    void set(int row, int col, T val);

    //! Add to matrix entry
    void add(int row, int col, T val);

    //! Multiply an entry with a scalar
    void multScalar(int row, int col, T val);

    //! ???
    void kroneckerKill(int row, int col);

    //! Multiply with a vector
    template <class VECtype>
    void mult(VECtype &ret, VECtype& x) const;

    //! Multiply with a vector
    template <class DiscFType, class DiscFuncType>
    void apply(const DiscFType &f, DiscFuncType &ret) const;

    //template <class DiscFuncType>
    //void apply(const DiscFuncType &f, DiscFuncType &ret) const;

    //! Multiply the transpose with a vector
    template <class DiscFuncType>
    void apply_t(const DiscFuncType &f, DiscFuncType &ret) const;

    //! Multiply with a vector
    template <class DiscFuncType>
    void operator () (const DiscFuncType &f, DiscFuncType &ret) const
    {
      apply(f,ret);
    };

    //! ???
    void print (std::ostream& s) const;

    //! ???
    void printReal (std::ostream& s) const;

    //! Makes a given row a unit row
    void unitRow(int row);

    //! Makes a given column a unit column
    void unitCol(int col);
  private:

  };


} // end namespace Sparselib

#include "spmatrix.cc"

#endif
