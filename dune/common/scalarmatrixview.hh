// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SCALARMATRIXVIEW_HH
#define DUNE_COMMON_SCALARMATRIXVIEW_HH

#include <cstddef>
#include <type_traits>
#include <ostream>

#include <dune/common/boundschecking.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/matvectraits.hh>
#include <dune/common/densematrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/scalarvectorview.hh>


namespace Dune {

namespace Impl {

  /**
      @addtogroup DenseMatVec
      @{
   */

  /*! \file
   * \brief Implements a scalar matrix view wrapper around an existing scalar.
   */

  /** \brief A wrapper making a scalar look like a matrix
   *
   * This stores a pointer to a scalar of type K and
   * provides the interface of a matrix with a single row
   * and column represented by the data behind the pointer.
   */
  template<class K>
  class ScalarMatrixView :
    public DenseMatrix<ScalarMatrixView<K>>
  {
    ScalarVectorView<K> data_;
    using Base = DenseMatrix<ScalarMatrixView<K>>;

    template <class>
    friend class ScalarMatrixView;
  public:

    //===== type definitions and constants

    //! The number of block levels we contain.
    //! This is always one for this type.
    constexpr static int blocklevel = 1;

    using size_type = typename Base::size_type;
    using row_type = typename Base::row_type;
    using row_reference = typename Base::row_reference;
    using const_row_reference = typename Base::const_row_reference;

    //! \brief The number of rows.
    //! This is always one for this type.
    constexpr static int rows = 1;
    //! \brief The number of columns.
    //! This is always one for this type.
    constexpr static int cols = 1;

    //===== constructors
    /** \brief Default constructor
     */
    constexpr ScalarMatrixView ()
      : data_()
    {}

    /** \brief Construct from a pointer to a scalar */
    ScalarMatrixView (K* p) :
      data_(p)
    {}

    //! Copy constructor
    ScalarMatrixView (const ScalarMatrixView &other) :
      Base(),
      data_(other.data_)
    {}

    //! Move constructor
    ScalarMatrixView (ScalarMatrixView &&other) :
      Base(),
      data_( other.data_ )
    {}

    //! Copy assignment operator
    ScalarMatrixView& operator= (const ScalarMatrixView& other)
    {
      data_ = other.data_;
      return *this;
    }

    template<class KK>
    ScalarMatrixView& operator= (const ScalarMatrixView<KK>& other)
    {
      data_ = other.data_;
      return *this;
    }

    //! Assignment operator from a scalar
    template<typename T,
      std::enable_if_t<std::is_convertible<T, K>::value, int> = 0>
    inline ScalarMatrixView& operator= (const T& k)
    {
      data_ = k;
      return *this;
    }

    // make this thing a matrix
    static constexpr size_type mat_rows() { return 1; }
    static constexpr size_type mat_cols() { return 1; }

    row_reference mat_access ([[maybe_unused]] size_type i)
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return data_;
    }

    const_row_reference mat_access ([[maybe_unused]] size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return data_;
    }
  }; // class ScalarMatrixView

  /** \brief Sends the matrix to an output stream */
  template<typename K>
  std::ostream& operator<< (std::ostream& s, const ScalarMatrixView<K>& a)
  {
    s << a[0][0];
    return s;
  }

  /** \brief Wrap a scalar as a 1-1-matrix */
  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asMatrix(T& t)
  {
    return ScalarMatrixView<T>{&t};
  }

  /** \brief Wrap a const scalar as a const 1-1-matrix */
  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asMatrix(const T& t)
  {
    return ScalarMatrixView<const T>{&t};
  }

  /** \brief Non-scalar types are assumed to be matrices, and simply forwarded */
  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  T& asMatrix(T& t)
  {
    return t;
  }

  /** \brief Non-scalar types are assumed to be matrices, and simply forwarded */
  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  const T& asMatrix(const T& t)
  {
    return t;
  }

  /** @} end documentation */

} // end namespace Impl

  template<class K>
  struct FieldTraits<Impl::ScalarMatrixView<K>> : public FieldTraits<std::remove_const_t<K>> {};

  template<class K>
  struct DenseMatVecTraits<Impl::ScalarMatrixView<K>>
  {
    using derived_type = Impl::ScalarMatrixView<K>;
    using row_type = Impl::ScalarVectorView<K>;
    using row_reference = row_type&;
    using const_row_reference = const row_type&;
    using value_type = std::remove_const_t<K>;
    using size_type = std::size_t;
  };


  template<class K>
  struct AutonomousValueType<Impl::ScalarMatrixView<K>>
  {
    using type = FieldMatrix<std::remove_const_t<K>,1,1>;
  };


} // end namespace Dune

#endif // DUNE_COMMON_SCALARMATRIXVIEW_HH
