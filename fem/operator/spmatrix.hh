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

    //! makes Matrix of zero length
    SparseRowMatrix();

    //! Copy Constructor
    SparseRowMatrix(const SparseRowMatrix<T> &S);

    /** \brief make matrix with 'rows' rows and 'cols' columns
     * \param rows Number of rows
     * \param cols number of columns
     * \param nz maximum number of nonzero values in each row
     * \param val Initialize all entries with this value
     */
    SparseRowMatrix(int rows, int cols, int nz, T val);

    //! free memory for values_ and col_
    ~SparseRowMatrix();

    /*******************************/
    /*  Access and info functions  */
    /*******************************/

    T&      val(int i) { return values_[i]; }


    //! \todo Please doc me!
    int colIndex(int row, int col);

    //! \todo Please doc me!
    const T&  val(int i) const { return values_[i]; }
    //! \todo Please doc me!
    const int&         row_ptr(int i) const { return rowptr_(i); }
    //! \todo Please doc me!
    const int&         col_ind(int i) const { return colind_(i);}

    //! \todo Please doc me!
    int dim(int i) const {return dim_[i];};

    //! \todo Please doc me!
    int size(int i) const {return dim_[i];};

    //! \todo Please doc me!
    int NumNonZeros() const {return nz_;};
    //! \todo Please doc me!
    int base() const {return base_;}

    //! Const index operator
    const T&  operator() (int i, int j) const;

    //! Set a matrix entry
    void set(int row, int col, T val);

    //! Add to a matrix entry
    void add(int row, int col, T val);

    //! \todo Please doc me!
    void kroneckerKill(int row, int col);

    //! \todo Please doc me!
    template <class VECtype>
    void mult(VECtype *ret, const VECtype* x) const;
    //! \todo Please doc me!
    void print (std::ostream& s) const;
    //! \todo Please doc me!
    void printReal (std::ostream& s) const;

  private:
    void unitRow(int row);
    void unitCol(int col);

  };


} // end namespace Sparselib

#include "spmatrix.cc"

#endif
