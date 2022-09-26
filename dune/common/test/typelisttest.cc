// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <dune/common/hybridutilities.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/typelist.hh>
#include <dune/common/typetraits.hh>

template<class... T>
int isTypeListByOverload(const Dune::TypeList<T...> *);

template<class T, class = void>
struct IsTypeListByOverload : std::false_type {};

template<class T>
struct IsTypeListByOverload
  <T, std::void_t<decltype(isTypeListByOverload(std::declval<const T*>()))> >
: std::true_type {};

template<class T>
struct IsTypeListBySpecialization : std::false_type {};

template<class... T>
struct IsTypeListBySpecialization<Dune::TypeList<T...> > : std::true_type {};

template<class TL>
void staticLiteralTests() {
  // check default-constructible
  TL tl1;
  TL tl2{};

  // check copy construction
  TL tl3 = tl1;
  TL tl4(tl1);

  // check move construction
  TL tl5 = std::move(tl1);
  TL tl6(std::move(tl2));

  // check copying
  tl1 = tl5;
  tl2 = { tl1 };

  // check moving
  tl3 = std::move(tl1);
  tl4 = { std::move(tl2) };

  // check literal type requirement
  constexpr TL tl7{};

  // specialization/overload resolution tests
  static_assert(IsTypeListBySpecialization<TL>::value,
                "TypeList cannot be recognized by class specialization");
  static_assert(IsTypeListByOverload<TL>::value,
                "TypeList cannot be recognized by function overload resolution");

  // avoid compiler warnings
  (void)tl1;
  (void)tl2;
  (void)tl3;
  (void)tl4;
  (void)tl5;
  (void)tl6;
  (void)tl7;

  // destructor checked on scope exit
}

static constexpr struct {} skipOverloadTest{};

template<class T>
void checkNonTypeList(decltype(skipOverloadTest))
{
  // make sure IsTypeList and IsEmptyTypeList reject non-typelists
  static_assert(!Dune::IsTypeList<T>::value,
                "IsTypeList accepts non-TypeList");
  static_assert(!Dune::IsEmptyTypeList<T>::value,
                "IsEmptyTypeList accepts non-TypeList");

  // specialization tests
  static_assert(!IsTypeListBySpecialization<T>::value,
                "Non-TypeList recognized as TypeList by class specialization");
}

template<class T>
void checkNonTypeList()
{
  checkNonTypeList<T>(skipOverloadTest);

  // overload resolution tests
  static_assert(!IsTypeListByOverload<T>::value,
                "Non-TypeList recognized as TypeList by function overload resolution");
}

void staticTests()
{
  {
    using TL = Dune::TypeList<>;
    static_assert(Dune::IsTypeList<TL>::value,
                  "TypeList not recognized by IsTypeList");
    static_assert(Dune::IsEmptyTypeList<TL>::value,
                  "Empty TypeList not recognized by IsEmptyTypeList");
    static_assert(Dune::TypeListSize<TL>::value == 0,
                  "Incorrect result of TypeListeSize");

    staticLiteralTests<TL>();
  }

  {
    using TL = Dune::TypeList<void>;
    static_assert(Dune::IsTypeList<TL>::value,
                  "TypeList not recognized by IsTypeList");
    static_assert(!Dune::IsEmptyTypeList<TL>::value,
                  "Nonempty TypeList declared empty by IsEmptyTypeList");
    static_assert(Dune::TypeListSize<TL>::value == 1,
                  "Incorrect result of TypeListeSize");
    static_assert(std::is_same<typename Dune::TypeListElement<0, TL>::type,
                               void>::value,
                  "TypeListElement returns wrong type");
    static_assert(std::is_same<Dune::TypeListEntry_t<0, TL>, void>::value,
                  "TypeListEntry_t returns wrong type");

    staticLiteralTests<TL>();
  }

  {
    using TL = Dune::TypeList<const int, int[10], int(int, int)>;
    static_assert(Dune::IsTypeList<TL>::value,
                  "TypeList not recognized by IsTypeList");
    static_assert(!Dune::IsEmptyTypeList<TL>::value,
                  "Nonempty TypeList declared empty by IsEmptyTypeList");
    static_assert(Dune::TypeListSize<TL>::value == 3,
                  "Incorrect result of TypeListeSize");

    static_assert(std::is_same<typename Dune::TypeListElement<0, TL>::type,
                               const int>::value,
                  "TypeListElement returns wrong type");
    static_assert(std::is_same<Dune::TypeListEntry_t<0, TL>, const int>::value,
                  "TypeListEntry_t returns wrong type");

    static_assert(std::is_same<typename Dune::TypeListElement<1, TL>::type,
                               int[10]>::value,
                  "TypeListElement returns wrong type");
    static_assert(std::is_same<Dune::TypeListEntry_t<1, TL>, int[10]>::value,
                  "TypeListEntry_t returns wrong type");

    static_assert(std::is_same<typename Dune::TypeListElement<2, TL>::type,
                               int(int, int)>::value,
                  "TypeListElement returns wrong type");
    static_assert(std::is_same<Dune::TypeListEntry_t<2, TL>,
                               int(int, int)>::value,
                  "TypeListEntry_t returns wrong type");

    staticLiteralTests<TL>();
  }

  // make sure IsTypeList and IsEmptyTypeList reject non-typelists
  checkNonTypeList<void>();
  checkNonTypeList<int>();
  // don't check tuple<>, that may actually be an implementation of TypeList<>
  checkNonTypeList<std::tuple<int> >();
  // `tuple<void>` is a complete, but noninstantiable type.  Attempting to use
  // an object of type `tuple<void>` as an argument to a function call
  // necessiates instantiation -- which is illegal even in an SFINAE context.
  // The instantiation is necessary to check for conversions (via conversion
  // operators and base classes) during overload resolution.  Even if the
  // signature of the function is of the form `f(const Expr<T>*)` for some
  // template parameter `T` and we call it as `f(declval<tuple<void>*>())` the
  // base classes `tuple<void>` must be determined to figure out whether
  // `tuple<void>*` can be converted to `Expr<T>*`.
  checkNonTypeList<std::tuple<void> >(skipOverloadTest);
}

struct NonConstructible {
  NonConstructible() = delete;
};

template<class TypeList>
auto getTypeInfos(TypeList typeList)
{
  using namespace Dune::Hybrid;

  std::vector<std::type_index> result;
  forEach(typeList, [&](auto metaType) {
    using type = typename decltype(metaType)::type;
    result.emplace_back(typeid (type));
  });
  return result;
}

template<class TypeList>
auto getMetaTypeInfos(TypeList typeList)
{
  using namespace Dune::Hybrid;

  std::vector<std::type_index> result;
  // The parens around `forEach` are needed to suppress ADL here to avoid
  // instantiation attempts for the member types of typeList
  (forEach)(typeList, [&](auto metaType) {
    result.emplace_back(typeid (metaType));
  });
  return result;
}

int main()
{
  staticTests();

  Dune::TestSuite test;

  auto typeList = Dune::TypeList<void, NonConstructible, int>{};
  auto expectedTypeInfoList = std::vector<std::type_index>{
    typeid (void), typeid (NonConstructible), typeid (int)
  };
  test.check(getTypeInfos(typeList) == expectedTypeInfoList)
    << "Iterating over TypeList yields unexpected type information";

  // This test also ensures that the type passed to the lamda in the
  // `forEach()` is indeed an instance of `MetaType`
  auto metaTypeList =
    Dune::TypeList<void, NonConstructible, std::tuple<void> >{};
  auto expectedMetaTypeInfoList = std::vector<std::type_index>{
    typeid (Dune::MetaType<void>),
    typeid (Dune::MetaType<NonConstructible>),
    typeid (Dune::MetaType<std::tuple<void> >),
  };
  // parens around `getMetaTypeInfos` needed to suppress ADL
  test.check((getMetaTypeInfos)(metaTypeList) == expectedMetaTypeInfoList)
    << "Iterating over TypeList yields unexpected MetaTypes";

  return test.exit();
}
