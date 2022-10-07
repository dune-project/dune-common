// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_DYNMATRIX_HH
#define DUNE_DYNMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>
#include <initializer_list>

#include <dune/common/boundschecking.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/densematrix.hh>
#include <dune/common/typetraits.hh>

namespace Dune
{

  /**
      @addtogroup DenseMatVec
      @{
   */

  /*! \file
   *  \brief This file implements a dense matrix with dynamic numbers of rows and columns.
   */

  template< class K > class DynamicMatrix;

  template< class K >
  struct DenseMatVecTraits< DynamicMatrix<K> >
  {
    typedef DynamicMatrix<K> derived_type;

    typedef DynamicVector<K> row_type;

    typedef row_type &row_reference;
    typedef const row_type &const_row_reference;

    typedef std::vector<K> container_type;
    typedef K value_type;
    typedef typename container_type::size_type size_type;
  };

  template< class K >
  struct FieldTraits< DynamicMatrix<K> >
  {
    typedef typename FieldTraits<K>::field_type field_type;
    typedef typename FieldTraits<K>::real_type real_type;
  };

  /** \brief Construct a matrix with a dynamic size.
   *
   * \tparam K is the field type (use float, double, complex, etc)
   */
  template<class K>
  class DynamicMatrix : public DenseMatrix< DynamicMatrix<K> >
  {
    std::vector< DynamicVector<K> > _data;
    typedef DenseMatrix< DynamicMatrix<K> > Base;
  public:
    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;
    typedef typename Base::row_type row_type;

    //===== constructors
    //! \brief Default constructor
    DynamicMatrix () {}

    //! \brief Constructor initializing the whole matrix with a scalar
    DynamicMatrix (size_type r, size_type c, value_type v = value_type() ) :
      _data(r, row_type(c, v) )
    {}

    /** \brief Constructor initializing the matrix from a list of vector
     */
    DynamicMatrix (std::initializer_list<DynamicVector<K>> const &ll)
      : _data(ll)
    {}


    template <class T,
              typename = std::enable_if_t<!Dune::IsNumber<T>::value && HasDenseMatrixAssigner<DynamicMatrix, T>::value>>
    DynamicMatrix(T const& rhs)
    {
      *this = rhs;
    }

    //==== resize related methods
    /**
     * \brief resize matrix to <code>r × c</code>
     *
     * Resize the matrix to <code>r × c</code>, using <code>v</code>
     * as the value of all entries.
     *
     * \warning All previous entries are lost, even when the matrix
     *          was not actually resized.
     *
     * \param r number of rows
     * \param c number of columns
     * \param v value of matrix entries
     */
    void resize (size_type r, size_type c, value_type v = value_type() )
    {
      _data.resize(0);
      _data.resize(r, row_type(c, v) );
    }

    //===== assignment
    // General assignment with resizing
    template <typename T,
              typename = std::enable_if_t<!Dune::IsNumber<T>::value>>
    DynamicMatrix& operator=(T const& rhs) {
      _data.resize(rhs.N());
      std::fill(_data.begin(), _data.end(), row_type(rhs.M(), K(0)));
      Base::operator=(rhs);
      return *this;
    }

    // Specialisation: scalar assignment (no resizing)
    template <typename T,
              typename = std::enable_if_t<Dune::IsNumber<T>::value>>
    DynamicMatrix& operator=(T scalar) {
      std::fill(_data.begin(), _data.end(), scalar);
      return *this;
    }

    //! Return transposed of the matrix as DynamicMatrix
    DynamicMatrix transposed() const
    {
      DynamicMatrix AT(this->M(), this->N());
      for( size_type i = 0; i < this->N(); ++i )
        for( size_type j = 0; j < this->M(); ++j )
          AT[j][i] = (*this)[i][j];
      return AT;
    }

    // make this thing a matrix
    size_type mat_rows() const { return _data.size(); }
    size_type mat_cols() const {
      assert(this->rows());
      return _data.front().size();
    }
    row_type & mat_access(size_type i) {
      DUNE_ASSERT_BOUNDS(i < _data.size());
      return _data[i];
    }
    const row_type & mat_access(size_type i) const {
      DUNE_ASSERT_BOUNDS(i < _data.size());
      return _data[i];
    }
  };

  /** @} end documentation */

} // end namespace

#endif
