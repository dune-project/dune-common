// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#include <config.h>

#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/typetree/treepath.hh>
#include <dune/common/version.hh>

int main(int argc, char** argv)
{

  using namespace Dune::Indices;
  Dune::TestSuite suite("Check treePath()");

  {
    auto root = Dune::TypeTree::treePath();
    constexpr auto path = Dune::TypeTree::treePath(_1,3,_2,5);

    static_assert(std::is_same<std::decay_t<decltype(path[_2])>,std::decay_t<decltype(_2)>>{},"wrong entry value");
    static_assert(path[_0] == 1,"wrong entry value");

    static_assert(std::is_same<std::decay_t<decltype(path[_1])>,std::size_t>{},"wrong entry type");
    static_assert(std::is_same<std::decay_t<decltype(path[_3])>,std::size_t>{},"wrong entry type");

    suite.check(path[_0] == 1);
    suite.check(path[_1] == 3);
    suite.check(path[_2] == 2);
    suite.check(path[_3] == 5);

    suite.check(path[1] == 3);
    suite.check(path[3] == 5);

    // Check free utility functions using qualified calls in
    // Dune:: and Dune::TypeTree:: and unqualified ADL calls

    suite.check(Dune::back(path) == Dune::TypeTree::back(path));
    suite.check(Dune::front(path) == Dune::TypeTree::front(path));
    suite.check(Dune::push_back(path, _3) == Dune::TypeTree::push_back(path, _3));
    suite.check(Dune::push_front(path, _3) == Dune::TypeTree::push_front(path, _3));
    suite.check(Dune::pop_back(path) == Dune::TypeTree::pop_back(path));
    suite.check(Dune::pop_front(path) == Dune::TypeTree::pop_front(path));
    suite.check(Dune::accumulate_back(path, _3) == Dune::TypeTree::accumulate_back(path, _3));
    suite.check(Dune::accumulate_front(path, _3) == Dune::TypeTree::accumulate_front(path, _3));
    suite.check(Dune::join(path, path) == Dune::TypeTree::join(path, path));
    suite.check(Dune::reverse(path) == Dune::TypeTree::reverse(path));

    suite.check(back(path) == Dune::TypeTree::back(path));
    suite.check(front(path) == Dune::TypeTree::front(path));
    suite.check(push_back(path, _3) == Dune::TypeTree::push_back(path, _3));
    suite.check(push_front(path, _3) == Dune::TypeTree::push_front(path, _3));
    suite.check(pop_back(path) == Dune::TypeTree::pop_back(path));
    suite.check(pop_front(path) == Dune::TypeTree::pop_front(path));
    suite.check(accumulate_back(path, _3) == Dune::TypeTree::accumulate_back(path, _3));
    suite.check(accumulate_front(path, _3) == Dune::TypeTree::accumulate_front(path, _3));
    suite.check(join(path, path) == Dune::TypeTree::join(path, path));
    suite.check(reverse(path) == Dune::TypeTree::reverse(path));

    suite.check(back(path) == 5);
    static_assert(back(push_back(path, _3)) == 3);
    suite.check(back(push_back(path, 3)) == 3);
    static_assert(back(pop_back(path)) == 2);
    suite.check(back(pop_back(path)) == 2);
    suite.check(back(pop_back(pop_back(path))) == 3);

    static_assert(front(path) == 1);
    static_assert(front(push_front(path,_0)) == 0);
    suite.check(front(push_front(path,0)) == 0);
    suite.check(front(pop_front(path)) == 3);
    static_assert(front(pop_front(pop_front(path))) == 2);
    suite.check(front(pop_front(pop_front(path))) == 2);

    suite.check(back(accumulate_back(path, 3)) == 8);
    suite.check(back(accumulate_back(path, _3)) == 8);
    static_assert(back(accumulate_back(pop_back(path), _3)) == _5);

    suite.check(front(accumulate_front(path, 3)) == 4);
    static_assert(front(accumulate_front(path, _3)) == _4);

    static_assert(pop_front(pop_front(pop_front(pop_front(path)))).size() == 0);
    static_assert(reverse(Dune::TypeTree::treePath()).size() == 0);

    constexpr auto rpath = reverse(path);
    suite.check(rpath[_0] == 5);
    suite.check(rpath[3] == 1);


    static_assert(join(root, path) == join(path, root));
    constexpr auto jpath = join(path, rpath);
    static_assert(jpath.max_size() == 8);
    static_assert(jpath == Dune::TypeTree::treePath(_1,3,_2,5,5,_2,3,_1));
    static_assert(join(path, Dune::TypeTree::treePath(5,_2), Dune::TypeTree::treePath(3, _1)) == Dune::TypeTree::treePath(_1,3,_2,5,5,_2,3,_1));
    suite.check(jpath[_3] == 5);
    suite.check(jpath[_4] == 5);
  }

  { // test the operator== for TreePath

    using Dune::TypeTree::treePath;

    suite.check(treePath(1,2,3) != treePath(1,2));
    suite.check(treePath(1,2,3) == treePath(1u,2u,3u));
    suite.check(treePath(1,2,3) != treePath(3,2,1));
    suite.check(treePath(1,2,3) == treePath(_1,_2,_3));
    suite.check(treePath(1,2,3) != treePath(_3,_2,_1));
    suite.check(treePath(_1,_2,_3) != treePath(_3,_2,_1));

    // check whether comparison can be used in constexpr context
    static_assert(treePath(_1,_2,_3) == treePath(_1,_2,_3));
    static_assert(treePath(_1,_2,_3) != treePath(_3,_2,_1));

    auto a = treePath(std::integral_constant<int,0>{}, std::integral_constant<int,1>{});
    auto b = treePath(std::integral_constant<std::size_t,0>{}, std::integral_constant<std::size_t,1>{});
    static_assert(decltype(a == b)::value);

    /* Note: It is not possible to check mixed integral constant arguments with
       the purely static overload of operator==

    auto c = treePath(std::integral_constant<std::size_t,0>{}, std::integral_constant<int,3>{});
    static_assert(decltype(a != c)::value);
    */
  }

  {
    using namespace Dune::TypeTree;

    auto path1 = 1_tp;
    static_assert(path1[_0] == 1);

    auto path123 = 123_tp;
    static_assert(path123[_0] == 123);
  }

  {
    using namespace Dune::TypeTree;
    using namespace Dune::Indices;
    auto tp = Dune::TypeTree::treePath(0, _1, 2, _3);

    auto os = std::stringstream();
    os << tp;
  }

  {
    using namespace Dune::TypeTree;
    using namespace Dune::Indices;
    auto tp = Dune::TypeTree::treePath(0, _1, 2, _3);

    suite.check(tp.get<0>() == 0);
    suite.check(tp.get<1>() == 1);
    suite.check(tp.get<2>() == 2);
    suite.check(tp.get<3>() == 3);

    auto [a, b, c, d] = tp;

    suite.check(a == 0);
    suite.check(b == 1);
    suite.check(c == 2);
    suite.check(d == 3);

    suite.check(std::is_same_v<decltype(a), std::size_t>);
    suite.check(std::is_same_v<decltype(b), Dune::index_constant<1>>);
    suite.check(std::is_same_v<decltype(c), std::size_t>);
    suite.check(std::is_same_v<decltype(d), Dune::index_constant<3>>);
  }

  {
    using namespace Dune::Indices;

    auto mi = Dune::HybridMultiIndex(3, _2, 1, _0);
    auto mi_size_t = Dune::HybridMultiIndex(std::size_t(3), std::integral_constant<std::size_t, 2>{}, std::size_t(1), std::integral_constant<std::size_t, 0>{});
    auto mi_signed = Dune::HybridMultiIndex(int(3), std::integral_constant<int, 2>{}, (signed char)(1), std::integral_constant<signed long, 0>{});

    suite.check(std::is_same_v<decltype(mi), decltype(mi_size_t)>);
    suite.check(std::is_same_v<decltype(mi), decltype(mi_signed)>);
    suite.check(mi == mi_size_t);
    suite.check(mi == mi_signed);
  }

  {
    using namespace Dune::Indices;

    auto t = std::make_tuple(_2, _1, std::size_t(0));
    auto mi_tuple = Dune::HybridMultiIndex(t);
    auto mi = Dune::HybridMultiIndex(_2, _1, std::size_t(0));

    suite.check(std::is_same_v<decltype(mi), decltype(mi_tuple)>);
    suite.check(mi == mi_tuple);
  }

  return suite.exit();
}
