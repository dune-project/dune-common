// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <algorithm>

#include <dune/common/densevector.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/unused.hh>

template<class Component, std::size_t Dim >
class MyVector;

namespace Dune
{
  template<class Component, std::size_t Dim>
  struct DenseMatVecTraits< MyVector<Component, Dim> >
  {
    using derived_type = MyVector<Component, Dim>;
    using value_type = Component;
    using size_type = std::size_t;
  };

  template<class Component, std::size_t Dim, int Size>
  struct IsFieldVectorSizeCorrect<MyVector<Component, Dim>, Size>
    : std::bool_constant<Dim == Size>
  {};
}

template<class Component, std::size_t Dim >
class MyVector : public Dune::DenseVector< MyVector<Component, Dim> >
{
 public:
  //MyVector (const Component& data) : data_(data) {}

  static constexpr std::size_t size () { return Dim; }

  Component& operator[] ( std::size_t i ) { return data_; }
  const Component& operator[] ( std::size_t i ) const { return data_; }
 protected:
  Component data_;
};

template<typename _Tp, typename _Up>
class my_is_assignable_helper
{
  template<typename _Tp1, typename _Up1,
           typename = decltype(std::declval<_Tp1>() = std::declval<_Up1>())>
  static std::true_type
  __test(int);

  template<typename, typename>
  static std::false_type
  __test(...);

 public:
  typedef decltype(__test<_Tp, _Up>(0)) type;
};

// move tests here in order to make sure that constexpr if blocks are
// really discarded.
template<class = void>
void test()
{
  // Pure 1d case.
  {
    using InnerFV = Dune::FieldVector<double, 1>;
    using MiddleFV = Dune::FieldVector<InnerFV, 1>;
    using OuterFV = Dune::FieldVector<MiddleFV, 1>;

    using MiddleMV = MyVector<InnerFV, 1>;
    using OuterMV = MyVector<MiddleMV, 1>;

    MiddleFV mfv;
    OuterMV mv;
    OuterFV fv;

    static_assert(std::is_convertible<OuterMV, OuterFV>::value,
                  "DenseVectors should be convertible.");
    fv = mv;

    if constexpr (std::is_assignable<MiddleFV, OuterMV>::value) {
      mfv = mv;
    }
  }

  // The following will trigger a problem in the DenseVector
  // operator=() which can be cured by first checking whether the
  // value_types are assignable.
  {
    using InnerFV = Dune::FieldVector<double, 2>;
    using MiddleFV = Dune::FieldVector<InnerFV, 1>;
    using OuterFV = Dune::FieldVector<MiddleFV, 1>;

    using MiddleMV = MyVector<InnerFV, 1>;
    using OuterMV = MyVector<MiddleMV, 1>;

    MiddleFV mfv;
    OuterMV mv;
    OuterFV fv;

    static_assert(std::is_convertible<OuterMV, OuterFV>::value,
                  "DenseVectors should be convertible.");
    fv = mv;

    if constexpr (std::is_assignable<MiddleFV, OuterMV>::value) {
      // This will fail. std::is_assignable is true here because of
      // the catch-all template ctor which allows ALL other dense-vectors.
      mfv = mv;
    }
  }

  {
    using InnerFV = Dune::FieldMatrix<double, 2, 2>;
    using MiddleFV = Dune::FieldVector<InnerFV, 1>;
    using OuterFV = Dune::FieldVector<MiddleFV, 1>;

    using MiddleMV = MyVector<InnerFV, 1>;
    using OuterMV = MyVector<MiddleMV, 1>;

    MiddleFV mfv;
    OuterMV mv;
    OuterFV fv;

    static_assert(std::is_assignable<OuterFV, OuterMV>::value,
                  "DenseVectors should be assignable.");
    fv = mv;

    if constexpr (std::is_assignable<MiddleFV, OuterMV>::value) {
      // This will fail. std::is_assignable is true here because of
      // the catch-all template ctor which allows ALL other dense-vectors.
      mfv = mv;
    }
  }
}

int main()
{
  try
  {
    test();
    return 0;
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
