// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: fmatrix.hh 6181 2010-10-13 18:53:40Z christi $
#ifndef DUNE_DYNMATRIX_HH
#define DUNE_DYNMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>

#include <dune/common/exceptions.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/densematrix.hh>
#include <dune/common/static_assert.hh>

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

    //==== resize related methods
    void resize (size_type r, size_type c, value_type v = value_type() )
    {
      _data.resize(0);
      _data.resize(r, row_type(c, v) );
    }

    //===== assignment
    using Base::operator=;

    // make this thing a matrix
    size_type mat_rows() const { return _data.size(); }
    size_type mat_cols() const {
      assert(this->rows());
      return _data.front().size();
    }
    row_type & mat_access(size_type i) { return _data[i]; }
    const row_type & mat_access(size_type i) const { return _data[i]; }
  };

  /** @} end documentation */

} // end namespace

#endif
