// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

#define EPS 1.0E-15

  /*****************************/
  /*  Constructor(s)           */
  /*****************************/
  template <class T>
  SparseRowMatrix<T>::SparseRowMatrix()
  {
    values_ = NULL;
    col_ = NULL;
    dim_[0] = 0;
    dim_[1] = 0;
    nz_ = 0;
  }

  template <class T>
  SparseRowMatrix<T>::~SparseRowMatrix()
  {
    if(values_) delete values_;
    if(col_) delete col_;
  }

  /***********************************/
  /*  Construct from storage vectors */
  /***********************************/

  template <class T>
  SparseRowMatrix<T>::SparseRowMatrix(int rows, int cols, int nz, T val)
  {
    dim_[0] = rows;
    dim_[1] = cols;
    nz_ = nz;

    values_ = new T [dim_[0]*nz_];
    col_ = new int [dim_[0]*nz_];

    for(int i=0; i<dim_[0]*nz_; i++)
    {
      values_[i] = val;
      col_[i] = -1;
    }
  }

  /***************************/
  /* Assignment operator...  */
  /***************************/
#if 0
  SparseRowMatrix& SparseRowMatrix::operator=(const SparseRowMatrix &R)
  {
    dim_[0] = R.dim_[0];
    dim_[1] = R.dim_[1];
    base_   = R.base_;
    nz_     = R.nz_;
    val_    = R.val_;
    rowptr_ = R.rowptr_;
    colind_ = R.colind_;
    return *this;
  }
  /***************************/
  /* newsize()               */
  /***************************/

  template <class T>
  SparseRowMatrix& SparseRowMatrix<T>::SparseRowMatrix::newsize(int M, int N, int nz)
  {
    dim_[0] = M;
    dim_[1] = N;
    nz_     = nz;
    val_.newsize(nz);
    rowptr_.newsize(M+1);
    colind_.newsize(nz);
    return *this;
  }

  /*********************/
  /*   Array access    */
  /*********************/
#endif

  template <class T>
  T SparseRowMatrix<T>::operator()(int row, int col) const
  {
    for (int i=0; i<nz_; i++)
    {
      if(col_[row*nz_ +i] == col)
      {
        return values_[row*nz_ +i];
      }
    }
    return 0.0;
  }

  template <class T>
  int SparseRowMatrix<T>::colIndex(int row, int col)
  {
    int whichCol = -1;
    int thisCol = 0;
    for(int i=0; i<nz_; i++)
    {
      thisCol = col_[row*nz_ +i];
      if(thisCol == -1) whichCol = i;
      if(col == thisCol) return i;
    }
    return whichCol;
  }

  template <class T>
  void SparseRowMatrix<T>::set(int row, int col, T val)
  {
    if(ABS(val) < EPS)
      return;

    int whichCol = colIndex(row,col);
    if(whichCol < 0)
    {
      std::cout << " error \n";
    }
    else
    {
      values_[row*nz_ + whichCol] = val;
      col_[row*nz_ + whichCol] = col;
    }
  }

  template <class T>
  void SparseRowMatrix<T>::add(int row, int col, T val)
  {
    int whichCol = colIndex(row,col);
    if(whichCol < 0)
    {
      std::cout << " error \n";
    }
    else
    {
      values_[row*nz_ + whichCol] += val;
      col_[row*nz_ + whichCol] = col;
    }
  }
  /***************************************/
  /*  Matrix-MV_Vector multiplication    */
  /***************************************/
  template <class T> template <class VECtype>
  void SparseRowMatrix<T>::mult(VECtype *ret, const VECtype *x) const
  {
    for(int row=0; row<dim_[0]; row++)
    {
      T& sum = ret[row];
      sum = 0.0;
      for(int col=0; col<nz_; col++)
      {
        int thisCol = row*nz_ + col;
        sum += values_[thisCol]*x[col_[thisCol]];
      }
    }

    return;
  }


  template <class T>
  void SparseRowMatrix<T>::print(std::ostream& s) const
  {
    for(int row=0; row<dim_[0]; row++)
    {
      for(int col=0; col<dim_[1]; col++)
      {
        s << (*this)(row,col) << " ";
      }
      s << "\n";
    }
    return;
  }

  template <class T>
  void SparseRowMatrix<T>::printReal(std::ostream& s) const
  {
    for(int row=0; row<dim_[0]; row++)
    {
      for(int col=0; col<nz_; col++)
      {
        s << values_[row*nz_ + col] << " ";
      }
      s << "\n";
    }
    return;
  }

  template <class T>
  void SparseRowMatrix<T>::kroneckerKill(int row, int col)
  {
    unitRow(row);
    //unitCol(col);
  }

  template <class T>
  void SparseRowMatrix<T>::unitRow(int row)
  {
    for(int i=1; i<nz_; i++)
    {
      values_[row*nz_ + i] = 0.0;
      col_[row*nz_ + i] = -1;
    }
    values_[row*nz_] = 1.0;
    col_[row*nz_] = row;
  }

  template <class T>
  void SparseRowMatrix<T>::unitCol(int col)
  {
    for(int i=0; i<dim_[0]; i++)
      if (i != col) set(i,col,0.0);
      else set(col,col,1.0);
  }

} // end namespace Dune
