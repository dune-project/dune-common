// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SCALARVECTORVIEW_HH
#define DUNE_COMMON_SCALARVECTORVIEW_HH

#include <cstddef>
#include <type_traits>
#include <istream>

#include <dune/common/densevector.hh>
#include <dune/common/fvector.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/matvectraits.hh>

namespace Dune {

namespace Impl {

  /** @addtogroup DenseMatVec
      @{
   */

  /*! \file
   * \brief Implements a scalar vector view wrapper around an existing scalar.
   */

  /** \brief A wrapper making a scalar look like a vector
   *
   * This stores a pointer to a scalar of type K and
   * provides the interface of a vector with a single
   * entry represented by the data behind the pointer.
   */
  template<class K>
  class ScalarVectorView :
    public DenseVector<ScalarVectorView<K>>
  {
    K* dataP_;
    using Base = DenseVector<ScalarVectorView<K>>;

    template <class>
    friend class ScalarVectorView;
  public:

    //! The size of this vector.
    constexpr static int dimension = 1;

    /** \brief The type used for array indices and sizes */
    using size_type = typename Base::size_type;

    /** \brief The type used for references to the vector entry */
    using reference = std::decay_t<K>&;

    /** \brief The type used for const references to the vector entry */
    using const_reference = const K&;

    //===== construction

    /** \brief Default constructor */
    constexpr ScalarVectorView ()
      : dataP_(nullptr)
    {}

    /** \brief Construct from a pointer to a scalar */
    ScalarVectorView (K* p) :
      dataP_(p)
    {}

    //! Copy constructor
    ScalarVectorView (const ScalarVectorView &other) :
      Base(),
      dataP_(other.dataP_)
    {}

    //! Move constructor
    ScalarVectorView (ScalarVectorView &&other) :
      Base(),
      dataP_( other.dataP_ )
    {}

    //! Copy assignment operator
    ScalarVectorView& operator= (const ScalarVectorView& other)
    {
      assert(dataP_);
      assert(other.dataP_);
      *dataP_ = *(other.dataP_);
      return *this;
    }

    template<class KK>
    ScalarVectorView& operator= (const ScalarVectorView<KK>& other)
    {
      assert(dataP_);
      assert(other.dataP_);
      *dataP_ = *(other.dataP_);
      return *this;
    }

    //! Assignment operator from a scalar
    template<typename T,
      std::enable_if_t<std::is_convertible<T, K>::value, int> = 0>
    inline ScalarVectorView& operator= (const T& k)
    {
      *dataP_ = k;
      return *this;
    }

    /** \brief Container size -- this is always 1 */
    static constexpr size_type size ()
    {
      return 1;
    }

    /** \brief Random access operator, actually disregards its argument */
    K& operator[] ([[maybe_unused]] size_type i)
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return *dataP_;
    }

    /** \brief Const random access operator, actually disregards its argument */
    const K& operator[] ([[maybe_unused]] size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return *dataP_;
    }
  }; // class ScalarVectorView

} // namespace Impl


  template< class K>
  struct DenseMatVecTraits< Impl::ScalarVectorView<K> >
  {
    using derived_type = Impl::ScalarVectorView<K>;
    using value_type = std::remove_const_t<K>;
    using size_type = std::size_t;
  };

  template< class K >
  struct FieldTraits< Impl::ScalarVectorView<K> > : public FieldTraits<std::remove_const_t<K>> {};

  template<class K>
  struct AutonomousValueType<Impl::ScalarVectorView<K>>
  {
    using type = FieldVector<std::remove_const_t<K>,1>;
  };

namespace Impl {

  /** \brief Read a ScalarVectorView from an input stream
   *  \relates ScalarVectorView
   *
   *  \note This operator is STL compliant, i.e., the content of v is only
   *        changed if the read operation is successful.
   *
   *  \param[in]  in  std :: istream to read from
   *  \param[out] v   ScalarVectorView to be read
   *
   *  \returns the input stream (in)
   */
  template<class K>
  inline std::istream &operator>> ( std::istream &in, ScalarVectorView<K> &v )
  {
    K w;
    if(in >> w)
      v = w;
    return in;
  }


  /** \brief Wrap a scalar as a 1-vector */
  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asVector(T& t)
  {
    return ScalarVectorView<T>{&t};
  }

  /** \brief Wrap a const scalar as a const 1-vector */
  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asVector(const T& t)
  {
    return ScalarVectorView<const T>{&t};
  }

  /** \brief Non-scalar types are assumed to be arrays, and simply forwarded */
  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  T& asVector(T& t)
  {
    return t;
  }

  /** \brief Non-scalar types are assumed to be arrays, and simply forwarded */
  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  const T& asVector(const T& t)
  {
    return t;
  }

} // end namespace Impl

} // end namespace Dune

#endif // DUNE_COMMON_SCALARVECTORVIEW_HH
