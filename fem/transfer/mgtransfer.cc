// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/fem/dofmanager.hh>
#include <dune/fem/lagrangebase.hh>
#include <dune/common/functionspace.hh>

template<class DiscFuncType>
template<class FunctionSpaceType>
void Dune::MGTransfer<DiscFuncType>::setup(const FunctionSpaceType& cS, const FunctionSpaceType& fS)
{
  coarseLevel = cS.level();
  fineLevel   = fS.level();

  //assert(fL == cL+1);
  //assert(level<grid_->maxlevel());

  typedef typename FunctionSpaceType::GridType GridType;

  int rows = fS.size();
  int cols = cS.size();

  GridType& grid = fS.getGrid();

  //
  matrix_.resize(rows, cols, GridType::dimension*10);
  matrix_.clear();

  typedef typename GridType::template Traits<0>::LevelIterator ElementIterator;
  typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

  ElementIterator cIt    = grid.template lbegin<0>(coarseLevel);
  ElementIterator cEndIt = grid.template lend<0>(coarseLevel);

  for (; cIt != cEndIt; ++cIt) {
    //std::cout << "Coarse: " << cIt->index() << "\n";

    const BaseFunctionSetType& coarseBaseSet = fS.getBaseFunctionSet( *cIt );
    const int numCoarseBaseFct = coarseBaseSet.getNumberOfBaseFunctions();


    typedef typename GridType::template Traits<0>::Entity EntityType;
    typedef typename EntityType::Traits::HierarchicIterator HierarchicIterator;

    HierarchicIterator fIt    = cIt->hbegin(fineLevel);
    HierarchicIterator fEndIt = cIt->hend(fineLevel);

    for (; fIt != fEndIt; ++fIt) {

      //std::cout << "Coarse: " << cIt->index() << "   fine: " << fIt->index() << "\n";

      const BaseFunctionSetType& fineBaseSet = fS.getBaseFunctionSet( *fIt );
      const int numFineBaseFct = fineBaseSet.getNumberOfBaseFunctions();

      for (int i=0; i<numCoarseBaseFct; i++) {

        int globalCoarse = fS.mapToGlobal(*cIt, i);

        for (int j=0; j<numFineBaseFct; j++) {

          int globalFine = fS.mapToGlobal(*fIt, j);


          //std::cout << "globalCoarse: " << globalCoarse
          //        << "   globalFine: " << globalFine << "\n";

          // Evaluate coarse grid base function at the location of the fine grid dof

          // first determine local fine grid dof position
          FieldVector<double, GridType::dimension> local = cIt->geometry().local(fIt->geometry()[j]);

          //std::cout << "finePos" << fIt->geometry()[j] << "\n";
          //std::cout << "locally" << local << "\n";

          // Evaluate coarse grid base function

          typename FunctionSpaceType::Range value;
          FieldVector<int, 0> diffVariable;
          coarseBaseSet.evaluate(i, diffVariable, local, value);

          //std::cout << "value: " << value << "\n";

          matrix_.set(globalFine, globalCoarse, value[0]);
        }


      }

    }

  }

}


template<class DiscFuncType>
void Dune::MGTransfer<DiscFuncType>::prolong(const DiscFuncType& f, DiscFuncType& t) const
{
  assert(t.getFunctionSpace().size()   == matrix_.rows());
  assert(f.getFunctionSpace().size() == matrix_.cols());

  typedef typename DiscFuncType::DofIteratorType DofIteratorType;
  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;

  DofIteratorType tIt = t.dbegin();
  DofIteratorType fIt = f.dbegin();

  for(int row=0; row<matrix_.rows(); row++) {

    *tIt = 0.0;
    /** \todo Remove casts */
    ColumnIterator cIt    = const_cast<SparseRowMatrix<double>&>(matrix_).template rbegin(row);
    ColumnIterator cEndIt = const_cast<SparseRowMatrix<double>&>(matrix_).template rend(row);

    for(; cIt!=cEndIt; ++cIt) {
      *tIt += *cIt * fIt[cIt.col()];
    }

    ++tIt;
  }
}

template<class DiscFuncType>
void Dune::MGTransfer<DiscFuncType>::restrict (const DiscFuncType & f, DiscFuncType& t) const
{
  assert(f.getFunctionSpace().size()   == matrix_.rows());
  assert(t.getFunctionSpace().size() == matrix_.cols());

  typedef typename DiscFuncType::DofIteratorType DofIteratorType;
  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;

  t.clear();

  DofIteratorType tIt = t.dbegin();
  DofIteratorType fIt = f.dbegin();


  for (int row=0; row<matrix_.rows(); row++) {

    /** \todo Remove casts */
    ColumnIterator cIt    = const_cast<SparseRowMatrix<double>&>(matrix_).template rbegin(row);
    ColumnIterator cEndIt = const_cast<SparseRowMatrix<double>&>(matrix_).template rend(row);

    for(; cIt!=cEndIt; ++cIt)
      tIt[cIt.col()] += fIt[row] * (*cIt);

  }

}


template<class DiscFuncType>
Dune::SparseRowMatrix<double> Dune::MGTransfer<DiscFuncType>::
galerkinRestrict(const Dune::SparseRowMatrix<double>& fineMat) const
{

  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;

  SparseRowMatrix<double> result(matrix_.cols() ,matrix_.cols() , fineMat.numNonZeros());

  for (int row=0; row<fineMat.rows(); row++) {

    ColumnIterator cIt    = const_cast<SparseRowMatrix<double>&>(fineMat).template rbegin(row);
    ColumnIterator cEndIt = const_cast<SparseRowMatrix<double>&>(fineMat).template rend(row);

    for(; cIt!=cEndIt; ++cIt) {

      double mvalue = *cIt;

      ColumnIterator tciIt    = const_cast<SparseRowMatrix<double>&>(matrix_).template rbegin(row);
      ColumnIterator tciEndIt = const_cast<SparseRowMatrix<double>&>(matrix_).template rend(row);

      for (; tciIt!=tciEndIt; ++tciIt) {

        double fac = mvalue* (*tciIt);

        ColumnIterator tcjIt    = const_cast<SparseRowMatrix<double>&>(matrix_).template rbegin(cIt.col());
        ColumnIterator tcjEndIt = const_cast<SparseRowMatrix<double>&>(matrix_).template rend(cIt.col());

        for (; tcjIt!=tcjEndIt; ++tcjIt) {

          //result.add( tciIt.col(), tcjIt.col(), fac* (*tcjIt));
          result.add( tcjIt.col(), tciIt.col(), fac* (*tcjIt));

        }

      }

    }


  }

  return result;
}
