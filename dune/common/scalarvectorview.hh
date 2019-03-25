// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_SCALARVECTORVIEW_HH
#define DUNE_COMMON_SCALARVECTORVIEW_HH

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <utility>
#include <initializer_list>
#include <algorithm>

#include "typetraits.hh"
#include "exceptions.hh"

#include "ftraits.hh"
#include "densevector.hh"
#include "unused.hh"
#include "boundschecking.hh"

#include <dune/common/math.hh>

namespace Dune {

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

  public:

    //! export size
    enum {
      //! The size of this vector.
      dimension = 1
    };

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

    ScalarVectorView (K* p) :
      dataP_(p)
    {}

    //! copy constructor
    ScalarVectorView ( const ScalarVectorView &other ) :
      Base(),
      dataP_( other.dataP_ )
    {}

    //! copy constructor
    ScalarVectorView (ScalarVectorView &&other) :
      Base(),
      dataP_( other.dataP_ )
    {}

    //! copy assignment operator
    ScalarVectorView& operator= (const ScalarVectorView& other)
    {
      assert(dataP_);
      assert(other.dataP_);
      *dataP_ = *(other.dataP_);
      return *this;
    }

    //! Assignment operator for scalar
    template<typename T,
      std::enable_if_t<std::is_convertible<T, K>::value, int> = 0>
    inline ScalarVectorView& operator= (const T& k)
    {
      *dataP_ = k;
      return *this;
    }

    //===== forward methods to container
    static constexpr size_type size () { return 1; }

    K & operator[](size_type i)
    {
      DUNE_UNUSED_PARAMETER(i);
      DUNE_ASSERT_BOUNDS(i == 0);
      return *dataP_;
    }
    const K & operator[](size_type i) const
    {
      DUNE_UNUSED_PARAMETER(i);
      DUNE_ASSERT_BOUNDS(i == 0);
      return *dataP_;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    operator K& () { return *dataP_; }

    /** \brief Const conversion operator */
    operator const K& () const { return *dataP_; }
  }; // class ScalarVectorView



  template< class K>
  struct DenseMatVecTraits< ScalarVectorView<K> >
  {
    using derived_type = ScalarVectorView<K>;
    using value_type = K;
    using size_type = std::size_t;
  };

  template< class K >
  struct FieldTraits< ScalarVectorView<K> > : public FieldTraits<K> {};

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



  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asVector(T& t)
  {
    return ScalarVectorView<T>{&t};
  }

  template<class T,
    std::enable_if_t<IsNumber<T>::value, int> = 0>
  auto asVector(const T& t)
  {
    return ScalarVectorView<const T>{&t};
  }

  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  T& asVector(T& t)
  {
    return t;
  }

  template<class T,
    std::enable_if_t<not IsNumber<T>::value, int> = 0>
  const T& asVector(const T& t)
  {
    return t;
  }





} // end namespace

#endif // DUNE_COMMON_SCALARVECTORVIEW_HH
