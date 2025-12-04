// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_TYPETREE_TEST_TESTTYPETREEUTILITIES_HH
#define DUNE_COMMON_TYPETREE_TEST_TESTTYPETREEUTILITIES_HH

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include <dune/common/indices.hh>
#include <dune/common/typetree/traversal.hh>


// Leaf node
template<class Payload>
class Leaf
{
  Payload value_;
public:

  Leaf() = default;

  Leaf(Payload value)
    : value_(value)
  {}

  // For testing
  int value() const { return value_; };
  int& value() { return value_; };
  auto name() const { return std::string("Leaf<") + std::to_string(value()) + std::string(">"); }

  // Node interface
  static constexpr auto degree() { return Dune::Indices::_0; }
};


// Uniform inner node with static degree
template<class Payload, class C, std::size_t n>
class UniformStaticInner
{
  Payload value_;
  std::array<C, n> children_;
public:

  UniformStaticInner() = default;

  UniformStaticInner(Payload value, C c)
    : value_(value)
  {
    for(std::size_t i=0; i<n; ++i)
      children_[i] = c;
  }

  UniformStaticInner(Payload value, C c, Dune::index_constant<n>)
    : UniformStaticInner(value, c)
  {}

  // For testing
  int value() const { return value_; };
  int& value() { return value_; };
  auto name() const { return std::string("UniformStaticInner"); }

  // Node interface
  auto degree() const { return n; }
  auto& child(std::size_t i) { return children_[i]; };
  const auto& child(std::size_t i) const { return children_[i]; };
};

// Uniform inner node with dynamic degree
template<class Payload, class C>
class UniformDynamicInner
{
  Payload value_;
  C prototype_;
  std::vector<C> children_;
public:

  UniformDynamicInner() = default;

  UniformDynamicInner(Payload value, C c, std::size_t n)
    : value_(value)
    , prototype_(c)
  {
    resize(n);
  }

  // For testing
  int value() const { return value_; };
  int& value() { return value_; };
  auto name() const { return std::string("UniformDynamicInner"); }
  void resize(std::size_t n) { children_.resize(n, prototype_); }

  // Node interface
  auto degree() const { return children_.size(); }
  auto& child(std::size_t i) { return children_[i]; };
  const auto& child(std::size_t i) const { return children_[i]; };
};

// Inner node with static degree
template<class Payload, class... CC>
class NonUniformInner
{
  Payload value_;
  std::tuple<CC...> children_;
public:

  NonUniformInner() = default;

  NonUniformInner(Payload value, CC... cc)
    : value_(value)
    , children_{std::move(cc)...}
  {}

  // For testing
  int value() const { return value_; };
  int& value() { return value_; };
  auto name() const { return std::string("NonUniformInner"); }

  // Node interface
  static constexpr auto degree() { return Dune::index_constant<sizeof...(CC)>{}; }
  template<std::size_t i>
  auto& child(Dune::index_constant<i>) { return std::get<i>(children_); }
  template<std::size_t i>
  const auto& child(Dune::index_constant<i>) const { return std::get<i>(children_); }
};



template<class Tree>
std::string treeName(const Tree& tree)
{
  using namespace std::string_literals;
  auto name = std::string{};
  Dune::TypeTree::forEachNode(tree,
    [&](auto&& node) { name += node.name() + "<"s; },
    [&](auto&& node) { name += node.name() + ","s; },
    [&](auto&& node) { name += ">"s; }
  );
  return name;
}

#endif // DUNE_COMMON_TYPETREE_TEST_TESTTYPETREEUTILITIES_HH
