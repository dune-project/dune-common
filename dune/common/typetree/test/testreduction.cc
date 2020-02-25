// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <tuple>

#include <dune/common/indices.hh>
#include <dune/common/test/testsuite.hh>

#include <dune/common/typetree/leafnode.hh>
#include <dune/common/typetree/powernode.hh>
#include <dune/common/typetree/compositenode.hh>
#include <dune/common/typetree/reduction.hh>

using namespace Dune;

class SimpleLeafNode :
    public Dune::TypeTree::LeafNode
{
  using Base = Dune::TypeTree::LeafNode;
public:

  SimpleLeafNode() :
    Base()
  {}
};

template<class C, std::size_t n>
class SimplePowerNode:
    public Dune::TypeTree::PowerNode<C,n>
{
  using Base = Dune::TypeTree::PowerNode<C,n>;
public:

  template<class... CC>
  SimplePowerNode(CC&&... cc) :
    Base(std::forward<CC>(cc)...)
  {}
};



template<class... C>
class SimpleCompositeNode:
    public Dune::TypeTree::CompositeNode<C...>
{
  using Base = Dune::TypeTree::CompositeNode<C...>;
public:

  template<class... CC>
  SimpleCompositeNode(CC&&... cc) :
    Base(std::forward<CC>(cc)...)
  {}
};


inline auto leafNode()
{
  return SimpleLeafNode{};
}

template<class... C>
auto compositeNode(C&&... c)
{
  return SimpleCompositeNode<std::decay_t<C>...>{std::forward<C>(c)...};
}

template<class C0, class... C>
auto powerNode(C0&& c0, C&&... c)
{
  return SimplePowerNode<std::decay_t<C0>, sizeof...(C)+1>{std::forward<C0>(c0), std::forward<C>(c)...};
}


int main()
{
  Dune::TestSuite test("tree reduction check");

  using Payload = std::size_t;

  auto tree = compositeNode(
                powerNode(
                  leafNode(),
                  leafNode(),
                  leafNode()
                ),
                leafNode());

  auto size = Dune::TypeTree::accumulateOverLeafs(tree, index_constant<0>{},
    [](auto&&, auto&&) { return index_constant<1>{}; },
    [](auto old, auto next) { return index_constant<old.value + next.value>{}; });

  auto nodeTuple = Dune::TypeTree::accumulateOverLeafs(tree, std::tuple<>{},
    [](auto const& node, auto&&) -> const auto& { return node; },
    [](auto const& old,  auto const& next) { return std::tuple_cat(old, std::make_tuple(next)); });

  static_assert(size.value == std::tuple_size<decltype(nodeTuple)>::value, "");


  auto size2 = Dune::TypeTree::accumulateOverLeafs(tree,
    [](auto&&, auto&&) { return index_constant<1>{}; },
    [](auto... ii) { return index_constant<(ii.value +...)>{}; });

  auto nodeTuple2 = Dune::TypeTree::accumulateOverLeafs(tree,
    [](auto const& node, auto&&) -> const auto& { return node; },
    [](auto const&... nodes) { return std::make_tuple(nodes...); });

  static_assert(size2.value == std::tuple_size<decltype(nodeTuple2)>::value, "");

  return test.exit();
}
