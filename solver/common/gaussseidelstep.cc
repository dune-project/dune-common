// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/fem/feop/spmatrix.hh>

template<class OperatorType, class DiscFuncType>
inline
DiscFuncType Dune::GaussSeidelStep<OperatorType, DiscFuncType>::getSol()
{
  return *(this->x_);
}

template<class OperatorType, class DiscFuncType>
inline
double Dune::GaussSeidelStep<OperatorType, DiscFuncType>::residual(int index) const
{
  /** \todo Const-casting because we don't have const iterators yet */
  //SparseRowMatrix<double>* mat = const_cast<SparseRowMatrix<double>* >(this->mat_->getMatrix());
  SparseRowMatrix<double>* mat = this->mat_;

  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;

  typedef typename DiscFuncType::DofIteratorType DofIterator;
  DofIterator dit = this->x_->dbegin();
  DofIterator rhsIt = this->rhs_->dbegin();

  /* The following loop computes
   * \f[ sum_i = \sum_{i \ne j} A_{ij}w_j \f]
   */
  double r = 0.0;
  ColumnIterator it    = mat->template rbegin(index);
  ColumnIterator endit = mat->template rend(index);

  for (; it!=endit; ++it)
    r += *it * dit[it.col()];

  r = rhsIt[index] - r;

  return r;
}

template<class OperatorType, class DiscFuncType>
inline
void Dune::GaussSeidelStep<OperatorType, DiscFuncType>::iterate()
{
  const SparseRowMatrix<double>* mat = this->mat_;   //->getMatrix();

  const int nDof = mat->size(0);

  int i;

  typedef typename DiscFuncType::DofIteratorType DofIterator;
  DofIterator dit = this->x_->dbegin();
  DofIterator rhsIt = this->rhs_->dbegin();

  for (i=0; i<nDof; i++) {

    if ((*this->dirichletNodes_)[i]) {

      dit[i] = rhsIt[i];
      continue;
    }

    double r = residual(i);

    dit[i] += r / (*mat)(i,i);

  }

}
