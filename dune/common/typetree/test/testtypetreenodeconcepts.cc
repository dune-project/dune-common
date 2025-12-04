// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#include <cstdlib>
#include <cstddef>

#include <dune/common/indices.hh>
#include <dune/common/typetree/nodeconcepts.hh>

// Bring concepts into scope
using Dune::TypeTree::Concept::TreeNode;
using Dune::TypeTree::Concept::InnerTreeNode;
using Dune::TypeTree::Concept::StaticDegreeInnerTreeNode;
using Dune::TypeTree::Concept::UniformInnerTreeNode;
using Dune::TypeTree::Concept::LeafTreeNode;
using namespace Dune::Indices;

// Leaf with static integral_constant 0
struct LeafStaticZero {
    static constexpr auto degree() { return _0; }
};
static_assert(TreeNode<LeafStaticZero>);
static_assert(LeafTreeNode<LeafStaticZero>);
static_assert(!UniformInnerTreeNode<LeafStaticZero>);
static_assert(!StaticDegreeInnerTreeNode<LeafStaticZero>);
static_assert(!InnerTreeNode<LeafStaticZero>);

// Leaf that reports non-zero degree but has no child -> should not be Inner nor Leaf
struct LeafNonZero {
    std::size_t degree() const { return 2; }
};
static_assert(TreeNode<LeafNonZero>);
static_assert(LeafTreeNode<LeafNonZero>);
static_assert(!UniformInnerTreeNode<LeafNonZero>);
static_assert(!StaticDegreeInnerTreeNode<LeafNonZero>);
static_assert(!InnerTreeNode<LeafNonZero>);

// degree() returns signed type (int) convertible to size_t
struct SignedDegreeReturn {
    int degree() const { return 1; }
};
static_assert(TreeNode<SignedDegreeReturn>);
static_assert(LeafTreeNode<SignedDegreeReturn>);
static_assert(!UniformInnerTreeNode<SignedDegreeReturn>);
static_assert(!StaticDegreeInnerTreeNode<SignedDegreeReturn>);
static_assert(!InnerTreeNode<SignedDegreeReturn>);

// Inner node with both static degree and both child APIs
struct InnerMixedChild {
    static constexpr auto degree() { return _3; }
    int child(std::size_t i) { return static_cast<int>(i); }
    template<std::size_t I>
    int child(Dune::index_constant<I>) { return static_cast<int>(I); }
};
static_assert(TreeNode<InnerMixedChild>);
static_assert(InnerTreeNode<InnerMixedChild>);
static_assert(UniformInnerTreeNode<InnerMixedChild>);
static_assert(StaticDegreeInnerTreeNode<InnerMixedChild>);
static_assert(!LeafTreeNode<InnerMixedChild>);

// Pure static-degree inner: only compile-time child access
struct InnerStaticOnly {
    static constexpr auto degree() { return _4; }
    template<std::size_t I>
    int child(Dune::index_constant<I>) { return static_cast<int>(I); }
};
static_assert(TreeNode<InnerStaticOnly>);
static_assert(InnerTreeNode<InnerStaticOnly>);
static_assert(StaticDegreeInnerTreeNode<InnerStaticOnly>);
static_assert(!UniformInnerTreeNode<InnerStaticOnly>);

// Runtime inner with const child (tests permissiveness of const)
struct InnerRuntimeConstChild {
    std::size_t degree() const { return 5; }
    int child(std::size_t i) const { return static_cast<int>(i); }
};
static_assert(TreeNode<InnerRuntimeConstChild>);
static_assert(InnerTreeNode<InnerRuntimeConstChild>);
static_assert(UniformInnerTreeNode<InnerRuntimeConstChild>);
static_assert(!StaticDegreeInnerTreeNode<InnerRuntimeConstChild>);

// Inner with heterogeneous child return types (template version returns long)
struct InnerHeteroReturn {
    static constexpr auto degree() { return _3; }
    int child(std::size_t i) { return static_cast<int>(i); }
    template<std::size_t I>
    long child(Dune::index_constant<I>) { return static_cast<long>(I); }
};
static_assert(TreeNode<InnerHeteroReturn>);
static_assert(InnerTreeNode<InnerHeteroReturn>);
static_assert(UniformInnerTreeNode<InnerHeteroReturn>);
static_assert(StaticDegreeInnerTreeNode<InnerHeteroReturn>);


// Inner node with static compile-time degree and uniform child API
struct UniformInner {
  // static degree known at compile-time, returns an integral_constant
  static constexpr auto degree() { return _3; }

  // uniform runtime child access
  int child(std::size_t i) { return static_cast<int>(i); }
};

static_assert(TreeNode<UniformInner>);
static_assert(InnerTreeNode<UniformInner>);
static_assert(UniformInnerTreeNode<UniformInner>);
static_assert(StaticDegreeInnerTreeNode<UniformInner>);
static_assert(!LeafTreeNode<UniformInner>);


// Static degree but runtime child only -> should still be StaticDegree + Uniform
struct StaticDegreeWithOnlyRuntimeChild {
    static constexpr auto degree() { return _2; }
    int child(std::size_t i) { return int(i); }
};
static_assert(TreeNode<StaticDegreeWithOnlyRuntimeChild>);
static_assert(InnerTreeNode<StaticDegreeWithOnlyRuntimeChild>);
static_assert(UniformInnerTreeNode<StaticDegreeWithOnlyRuntimeChild>);
static_assert(StaticDegreeInnerTreeNode<StaticDegreeWithOnlyRuntimeChild>);

// Static degree without static
struct StaticDegreeWithNoStaticDegree {
    auto degree() const { return _2; }
    template<std::size_t I>
    auto child(Dune::index_constant<I>) { return int(I); }
};
static_assert(TreeNode<StaticDegreeWithNoStaticDegree>);
static_assert(!InnerTreeNode<StaticDegreeWithNoStaticDegree>);
static_assert(!UniformInnerTreeNode<StaticDegreeWithNoStaticDegree>);
static_assert(!StaticDegreeInnerTreeNode<StaticDegreeWithNoStaticDegree>);
static_assert(LeafTreeNode<StaticDegreeWithNoStaticDegree>);

// Inner node with static degree == 0 (should not model StaticDegreeInnerTreeNode)
struct StaticDegreeZeroInner {
  static constexpr auto degree() { return _0; }

  template<std::size_t I>
  int child(Dune::index_constant<I>) { return static_cast<int>(I); }
};

static_assert(TreeNode<StaticDegreeZeroInner>);
static_assert(!StaticDegreeInnerTreeNode<StaticDegreeZeroInner>);
static_assert(!InnerTreeNode<StaticDegreeZeroInner>);
static_assert(!UniformInnerTreeNode<StaticDegreeZeroInner>); // no runtime child
static_assert(LeafTreeNode<StaticDegreeZeroInner>);

// Static degree but missing some child overloads via requires -> should fail StaticDegreeInnerTreeNode
struct IncompleteStaticDegree {
    static constexpr auto degree() { return _3; }
    template<std::size_t I>
        requires (I < 2) // Missing child for I==2
    int child(Dune::index_constant<I>) { return int(I); }
};
static_assert(TreeNode<IncompleteStaticDegree>);
static_assert(!InnerTreeNode<IncompleteStaticDegree>);
static_assert(!StaticDegreeInnerTreeNode<IncompleteStaticDegree>);
static_assert(!UniformInnerTreeNode<IncompleteStaticDegree>);

// Node with static degree zero but providing a (unused) child -> should become inner
struct ZeroDegreeWithChild {
    static constexpr auto degree() { return _0; }
    int child(std::size_t) { return 0; }
};
static_assert(TreeNode<ZeroDegreeWithChild>);
static_assert(InnerTreeNode<ZeroDegreeWithChild>);
static_assert(!LeafTreeNode<ZeroDegreeWithChild>);
static_assert(UniformInnerTreeNode<ZeroDegreeWithChild>);
static_assert(!StaticDegreeInnerTreeNode<ZeroDegreeWithChild>);

// Runtime degree but compile-time child only -> leaf node as no runtime child access
struct RuntimeDegreeCompileTimeChild {
    std::size_t degree() const { return 3; }
    template<std::size_t I>
    int child(Dune::index_constant<I>) { return int(I); }
};
static_assert(TreeNode<RuntimeDegreeCompileTimeChild>);
static_assert(LeafTreeNode<RuntimeDegreeCompileTimeChild>);
static_assert(!InnerTreeNode<RuntimeDegreeCompileTimeChild>);
static_assert(!StaticDegreeInnerTreeNode<RuntimeDegreeCompileTimeChild>);
static_assert(!UniformInnerTreeNode<RuntimeDegreeCompileTimeChild>);

// degree() returns bool convertible to size_t
struct BoolDegree {
    bool degree() const { return false; }
};
static_assert(TreeNode<BoolDegree>);
static_assert(LeafTreeNode<BoolDegree>);
static_assert(!InnerTreeNode<BoolDegree>);

// Inner node with dependent qualified child return type
struct QualifiedDependentChild {
    constexpr static auto degree() { return _2; }
    template<class T=void>
    int child(std::size_t i) { return int(i); }
    template<std::size_t I>
    int child(Dune::index_constant<I>) const { return int(I); }
};
static_assert(TreeNode<QualifiedDependentChild>);
static_assert(InnerTreeNode<QualifiedDependentChild>);
static_assert(UniformInnerTreeNode<QualifiedDependentChild>);
static_assert(!StaticDegreeInnerTreeNode<QualifiedDependentChild>);

static_assert(TreeNode<QualifiedDependentChild&>);
static_assert(InnerTreeNode<QualifiedDependentChild&>);
static_assert(UniformInnerTreeNode<QualifiedDependentChild&>);
static_assert(!StaticDegreeInnerTreeNode<QualifiedDependentChild&>);

static_assert(TreeNode<const QualifiedDependentChild&>);
static_assert(InnerTreeNode<const QualifiedDependentChild&>);
static_assert(!UniformInnerTreeNode<const QualifiedDependentChild&>);
static_assert(StaticDegreeInnerTreeNode<const QualifiedDependentChild&>); // because const version has only static child


// degree() returning reference
struct DegreeReturnsRef {
    std::size_t d = 0;
    std::size_t const& degree() const { return d; }
};
static_assert(TreeNode<DegreeReturnsRef>);
static_assert(LeafTreeNode<DegreeReturnsRef>);
static_assert(!InnerTreeNode<DegreeReturnsRef>);

// degree() returning volatile qualified
struct VolatileDegree {
    mutable volatile std::size_t d = 0;
    volatile std::size_t& degree() const { return d; }
};
static_assert(TreeNode<VolatileDegree>);
static_assert(LeafTreeNode<VolatileDegree>);
static_assert(!InnerTreeNode<VolatileDegree>);

// Inner with child returning different types depending on index via if constexpr
struct InnerVariantReturn {
    static constexpr auto degree() { return _3; }
    template<std::size_t I>
    auto child(Dune::index_constant<I>) {
        if constexpr (I==0) return 0;
        else if constexpr (I==1) return 1.0;
        else return 'c';
    }
};
static_assert(TreeNode<InnerVariantReturn>);
static_assert(InnerTreeNode<InnerVariantReturn>);
static_assert(StaticDegreeInnerTreeNode<InnerVariantReturn>);
static_assert(!UniformInnerTreeNode<InnerVariantReturn>);

// Inner node with static degree returning plain size_t (not a compile-time constant type for the trait)
struct WrongStaticDegreeReturn {
  static constexpr std::size_t degree() { return 2; } // returns size_t, not an integral_constant

  template<std::size_t I>
  int child(Dune::index_constant<I>) { return static_cast<int>(I); }
};

static_assert(TreeNode<WrongStaticDegreeReturn>);
static_assert(!StaticDegreeInnerTreeNode<WrongStaticDegreeReturn>);
static_assert(!InnerTreeNode<WrongStaticDegreeReturn>);

// Node with non-const degree() (should not model TreeNode)
struct NonConstDegree {
  std::size_t degree() { return 1; } // missing const
  template<std::size_t I>
  int child(Dune::index_constant<I>) { return static_cast<int>(I); }
};

static_assert(!TreeNode<NonConstDegree>);

int main() {
    return EXIT_SUCCESS;
}
