// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/fem/dofmanager.hh>
#include <dune/fem/lagrangebase.hh>
#include <dune/common/functionspace.hh>

template<class DiscFuncType>
void Dune::MGTransfer<DiscFuncType>::setup(const FunctionSpaceType& fS, int cL, int fL)
{
  coarseLevel = cL;
  fineLevel   = fL;

  assert(fL == cL+1);
  //assert(level<grid_->maxlevel());

  typedef typename FunctionSpaceType::GridType GridType;

  int rows = fS.size(coarseLevel);
  int cols = fS.size(fineLevel);

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

          matrix_.set(globalCoarse, globalFine, value[0]);
        }


      }

    }

  }

  //matrix_.print(std::cout);
}


template<class DiscFuncType>
void Dune::MGTransfer<DiscFuncType>::restrict (const DiscFuncType & f, DiscFuncType& t) const
{
  assert(t.getFunctionSpace().size(coarseLevel) == matrix_.rows());
  assert(f.getFunctionSpace().size(fineLevel)   == matrix_.cols());

  typedef typename DiscFuncType::DofIteratorType DofIteratorType;
  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;


  DofIteratorType tIt = t.dbegin( coarseLevel );
  DofIteratorType fIt = f.dbegin( fineLevel );

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
void Dune::MGTransfer<DiscFuncType>::prolong(const DiscFuncType& f, DiscFuncType& t) const
{
  assert(f.getFunctionSpace().size(coarseLevel) == matrix_.rows());
  assert(t.getFunctionSpace().size(fineLevel)   == matrix_.cols());

  typedef typename DiscFuncType::DofIteratorType DofIteratorType;
  typedef typename SparseRowMatrix<double>::ColumnIterator ColumnIterator;

  t.clearLevel(fineLevel);

  DofIteratorType tIt = t.dbegin( fineLevel );
  DofIteratorType fIt = f.dbegin( coarseLevel );


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
  //return matrix_.applyFromLeftAndRightTo(fineMat);

  // Hack: We need the transposed matrix

  SparseRowMatrix<double> transpose(matrix_.cols(), matrix_.rows(), matrix_.numNonZeros());

  for (int row=0; row<matrix_.rows(); row++) {

    /** \todo Remove casts */
    ColumnIterator cIt    = const_cast<SparseRowMatrix<double>&>(matrix_).template rbegin(row);
    ColumnIterator cEndIt = const_cast<SparseRowMatrix<double>&>(matrix_).template rend(row);

    for(; cIt!=cEndIt; ++cIt)
      transpose.set(cIt.col(), row, *cIt);

  }

  SparseRowMatrix<double> result(matrix_.rows() ,matrix_.rows() , fineMat.numNonZeros());

  //for (v=FIRSTVECTOR(FineGrid); v!=NULL; v=SUCCVC(v))
  for (int row=0; row<fineMat.rows(); row++) {

    //         for (m=VSTART(v); m!=NULL; m=MNEXT(m)) {
    //             w = MDEST(m);
    ColumnIterator cIt    = const_cast<SparseRowMatrix<double>&>(fineMat).template rbegin(row);
    ColumnIterator cEndIt = const_cast<SparseRowMatrix<double>&>(fineMat).template rend(row);

    for(; cIt!=cEndIt; ++cIt) {

      //mvalue = MVALUE(m,mc);
      double mvalue = *cIt;

      ColumnIterator tciIt    = transpose.template rbegin(row);
      ColumnIterator tciEndIt = transpose.template rend(row);

      //for (im=VISTART(v); im!= NULL; im = NEXT(im)) {
      for (; tciIt!=tciEndIt; ++tciIt) {

        //fac = mvalue*MVALUE(im,0);
        double fac = mvalue* (*tciIt);

        ColumnIterator tcjIt    = transpose.template rbegin(cIt.col());
        ColumnIterator tcjEndIt = transpose.template rend(cIt.col());

        //for (jm=VISTART(v); jm!= NULL; jm = NEXT(jm)) {
        for (; tcjIt!=tcjEndIt; ++tcjIt) {

          //                             jv = MDEST(jm);
          //                             cm = GetMatrix(iv,jv);
          //                             MVALUE(cm,mc) +=
          //                                 fac * MVALUE(jm,0);
          result.add( tciIt.col(), tcjIt.col(), fac* (*tcjIt));
        }

      }

    }


  }

  return result;
}
