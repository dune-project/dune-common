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
    typedef T Ttype; //! remember the value type

  private:
    T* values_;     //! data values (nz_ elements)
    int* col_;      //! row_ptr (dim_[0]+1 elements)
    int dim_[2];    //! dim_[0] x dim_[1] Matrix
    int nz_;        //! number of nonzeros per row

  public:
    SparseRowMatrix(); //! makes Matrix of zero length
    SparseRowMatrix(const SparseRowMatrix<T> &S); //! Copy Constructor

    //! make matrix with 'rows' rows and 'cols' columns,
    //! maximum 'nz' non zero values in each row
    //! and intialize all values with 'val'
    SparseRowMatrix(int rows, int cols, int nz, T val);

    //! free memory for values_ and col_
    ~SparseRowMatrix();

    /*******************************/
    /*  Access and info functions  */
    /*******************************/
    T&      val(int i) { return values_[i]; }
    int colIndex(int row, int col);

    const T&  val(int i) const { return values_[i]; }
    //const int&         row_ptr(int i) const { return rowptr_(i); }
    //const int&         col_ind(int i) const { return colind_(i);}

    int dim(int i) const {return dim_[i];};
    int size(int i) const {return dim_[i];};
    int NumNonZeros() const {return nz_;};
    //int base() const {return base_;}
    T operator() (int i, int j) const;

    void set(int row, int col, T val);
    void add(int row, int col, T val);
    void multScalar(int row, int col, T val);

    void kroneckerKill(int row, int col);

    template <class VECtype>
    void mult(VECtype &ret, VECtype& x) const;

    template <class DiscFType, class DiscFuncType>
    void apply(const DiscFType &f, DiscFuncType &ret) const;

    //template <class DiscFuncType>
    //void apply(const DiscFuncType &f, DiscFuncType &ret) const;

    template <class DiscFuncType>
    void apply_t(const DiscFuncType &f, DiscFuncType &ret) const;

    template <class DiscFuncType>
    void operator () (const DiscFuncType &f, DiscFuncType &ret) const
    {
      apply(f,ret);
    };

    void print (std::ostream& s) const;
    void printReal (std::ostream& s) const;

    void unitRow(int row);
    void unitCol(int col);
  private:

  };


} // end namespace Sparselib

#include "spmatrix.cc"

#endif
