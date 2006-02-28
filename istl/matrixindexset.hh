// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MATRIX_INDEX_SET_HH
#define DUNE_MATRIX_INDEX_SET_HH

#include <vector>

namespace Dune {


  /** \brief Stores the nonzero entries in a sparse matrix */
  class MatrixIndexSet
  {

  public:

    /** \brief Default constructor */
    MatrixIndexSet() : rows_(0), cols_(0)
    {}

    /** \brief Constructor setting the matrix size */
    MatrixIndexSet(int rows, int cols) : rows_(rows), cols_(cols) {
      indices.resize(rows_);
    }

    /** \brief Reset the size of an index set */
    void resize(int rows, int cols) {
      rows_ = rows;
      cols_ = cols;
      indices.resize(rows_);
    }

    /** \brief Add an index to the index set */
    void add(int i, int j) {

      if (!containsSorted(indices[i], j))
        insertSorted(indices[i], j);

    }

    /** \brief Adds a dummy entry to each empty row
     * \todo Remove this once BCRSMatrix allows empty rows
     */
    void pad() {
      for (int i=0; i<rows_; i++)
        if (indices[i].size()==0)
          add(i,0);
    }

    /** \brief Return the number of entries */
    int size() const {
      int entries = 0;
      for (int i=0; i<rows_; i++)
        entries += indices[i].size();

      return entries;
    }

    /** \brief Return the number of rows */
    int rows() const {return rows_;}


    /** \brief Return the number of entries in a given row */
    int rowsize(int row) const {return indices[row].size();}

    /** \brief Import all nonzero entries of a sparse matrix into the index set
     * \tparam MatrixType Needs to be BCRSMatrix<...>
     */
    template <class MatrixType>
    void import(const MatrixType& m, int rowOffset=0, int colOffset=0) {

      typedef typename MatrixType::row_type RowType;
      typedef typename RowType::ConstIterator ColumnIterator;

      for (int rowIdx=0; rowIdx<m.N(); rowIdx++) {

        const RowType& row = m[rowIdx];

        ColumnIterator cIt    = row.begin();
        ColumnIterator cEndIt = row.end();

        for(; cIt!=cEndIt; ++cIt)
          add(rowIdx+rowOffset, cIt.index()+colOffset);

      }

    }

    /** \brief Initializes a BCRSMatrix with the indices contained
        in this MatrixIndexSet
        \tparam MatrixType Needs to be BCRSMatrix<...>
     */
    template <class MatrixType>
    void exportIdx(MatrixType& m) const {

      MatrixType matrix(rows_, cols_, MatrixType::random);

      for (int i=0; i<rows_; i++) {
        matrix.setrowsize(i, indices[i].size());
      }

      matrix.endrowsizes();

      for (int i=0; i<rows_; i++) {
        for (unsigned int j=0; j<indices[i].size(); j++)
          matrix.addindex(i, indices[i][j]);

      }

      matrix.endindices();

      /** \todo This copying could be omitted if there was a resize method for BCRSMatrix */
      m = matrix;
    }

  private:

    /** \todo Doesn't use the fact that the array is sorted! */
    bool containsSorted(const std::vector<int>& nb, int idx) {

      for (unsigned int i=0; i<nb.size(); i++)
        if (nb[i]==idx)
          return true;

      return false;
    }

    void insertSorted(std::vector<int>& nb, int idx) {

      unsigned int i;
      // Find correct slot for insertion
      for (i=0; i<nb.size(); i++)
        if (nb[i] >= idx)
          break;

      // Insert
      nb.push_back(0);
      for (unsigned int j=nb.size()-1; j>i; j--)
        nb[j] = nb[j-1];

      nb[i] = idx;

    }

    std::vector<std::vector<int> > indices;

    int rows_, cols_;

  };


} // end namespace Dune

#endif
