// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


template<class DiscFuncType>
void Dune::TruncatedMGTransfer<DiscFuncType>::prolong(const DiscFuncType& f, DiscFuncType& t,
                                                      const Dune::BitField& critical) const
{
  if (f.size() != this->matrix_.M())
    DUNE_THROW(Dune::Exception, "Number of entries in the coarse grid vector is not equal "
               << "to the number of columns of the interpolation matrix!");

  t.resize(this->matrix_.N());

  typedef typename DiscFuncType::Iterator Iterator;
  typedef typename DiscFuncType::ConstIterator ConstIterator;

  typedef typename OperatorType::row_type RowType;
  typedef typename RowType::ConstIterator ColumnIterator;

  Iterator tIt      = t.begin();
  ConstIterator fIt = f.begin();



  for(int rowIdx=0; rowIdx<this->matrix_.N(); rowIdx++) {

    const RowType& row = this->matrix_[rowIdx];

    *tIt = 0.0;
    ColumnIterator cIt    = row.begin();
    ColumnIterator cEndIt = row.end();

    for(; cIt!=cEndIt; ++cIt) {

      // The following lines are a matrix-vector loop, but rows belonging
      // to critical dofs are left out
      for (int i=0; i<blocksize; i++) {

        for (int j=0; j<blocksize; j++) {

          if (!critical[rowIdx*blocksize + i])
            (*tIt)[i] += (*cIt)[i][j] * f[cIt.index()][j];

        }

      }

    }

    ++tIt;
  }

}

template<class DiscFuncType>
void Dune::TruncatedMGTransfer<DiscFuncType>::restrict (const DiscFuncType & f, DiscFuncType& t,
                                                        const Dune::BitField& critical) const
{
  if (f.size() != this->matrix_.N())
    DUNE_THROW(Dune::Exception, "Fine grid vector has " << f.size() << " entries "
                                                        << "but the interpolation matrix has " << this->matrix_.N() << " rows!");

  t.resize(this->matrix_.M());
  t = 0;

  typedef typename DiscFuncType::Iterator Iterator;
  typedef typename DiscFuncType::ConstIterator ConstIterator;

  typedef typename OperatorType::row_type RowType;
  typedef typename RowType::ConstIterator ColumnIterator;

  Iterator tIt      = t.begin();
  ConstIterator fIt = f.begin();

  for (int rowIdx=0; rowIdx<this->matrix_.N(); rowIdx++) {

    const RowType& row = this->matrix_[rowIdx];

    ColumnIterator cIt    = row.begin();
    ColumnIterator cEndIt = row.end();

    for(; cIt!=cEndIt; ++cIt) {

      // The following lines are a matrix-vector loop, but rows belonging
      // to critical dofs are left out
      typename DiscFuncType::block_type& tEntry = t[cIt.index()];
      for (int i=0; i<blocksize; i++) {

        for (int j=0; j<blocksize; j++) {

          if (!critical[rowIdx*blocksize + j])
            tEntry[i] += (*cIt)[j][i] * f[rowIdx][j];

        }

      }

    }

  }

}


template<class DiscFuncType>
void Dune::TruncatedMGTransfer<DiscFuncType>::
galerkinRestrict(const OperatorType& fineMat, OperatorType& coarseMat,
                 const Dune::BitField& critical) const
{

  if (recompute_ != NULL && recompute_->size() != this->matrix_.M())
    DUNE_THROW(Exception, "The size of the recompute_-bitfield doesn't match the "
               << "size of the coarse grid space!");

  // ////////////////////////
  // Nonsymmetric case
  // ////////////////////////
  typedef typename OperatorType::row_type RowType;
  typedef typename RowType::Iterator ColumnIterator;
  typedef typename RowType::ConstIterator ConstColumnIterator;

  // Clear coarse matrix
  if (recompute_ == NULL)
    coarseMat = 0;
  else {
    for (int i=0; i<coarseMat.N(); i++) {

      RowType& row = coarseMat[i];

      // Loop over all columns of the stiffness matrix
      ColumnIterator m    = row.begin();
      ColumnIterator mEnd = row.end();

      for (; m!=mEnd; ++m) {

        int w = m.index();

        if ((*recompute_)[i] || (*recompute_)[m.index()])
          *m = 0;

      }

    }

  }

  // Loop over all rows of the stiffness matrix
  for (int v=0; v<fineMat.N(); v++) {

    const RowType& row = fineMat[v];

    // Loop over all columns of the stiffness matrix
    ConstColumnIterator m    = row.begin();
    ConstColumnIterator mEnd = row.end();

    for (; m!=mEnd; ++m) {

      int w = m.index();

      // Loop over all coarse grid vectors iv that have v in their support
      ConstColumnIterator im    = this->matrix_[v].begin();
      ConstColumnIterator imEnd = this->matrix_[v].end();
      for (; im!=imEnd; ++im) {

        int iv = im.index();



        // Loop over all coarse grid vectors jv that have w in their support
        ConstColumnIterator jm = this->matrix_[w].begin();
        ConstColumnIterator jmEnd = this->matrix_[w].end();

        for (; jm!=jmEnd; ++jm) {

          int jv = jm.index();

          if (recompute_ && !((*recompute_)[iv]) && !((*recompute_)[jv]))
            continue;

          MatrixBlock& cm = coarseMat[iv][jv];

          // Compute im * m * jm, but omitting the critical entries
          for (int i=0; i<blocksize; i++) {

            for (int j=0; j<blocksize; j++) {

              double sum = 0.0;

              for (int k=0; k<blocksize; k++) {

                for (int l=0; l<blocksize; l++) {
                  // Truncated Multigrid:  Omit coupling if at least
                  // one of the two vectors is critical
                  if (!critical[v*blocksize+k] && !critical[w*blocksize+l]) {
                    sum += (*im)[k][i] * (*m)[k][l] * (*jm)[l][j];

                  }

                }

              }

              cm[i][j] += sum;

            }

          }

        }
      }
    }
  }

}
