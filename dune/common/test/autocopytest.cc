// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <type_traits>
#include <utility>
#include <vector>

#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>

template<class T>
constexpr auto doAutoCopy(T &&v)
{
  return Dune::autoCopy(std::forward<T>(v));
}

// an example expression object that evaluates to int(0)
struct ZeroExpr {
  constexpr operator int() const volatile { return 0; }
};

namespace Dune {
  template<>
  struct AutonomousValueType<ZeroExpr> { using type = int; };

  // doAutoCopy should not pick up this overload
  constexpr auto autoCopy(ZeroExpr) = delete;
} // namespace Dune

int main()
{

  {
    std::vector<bool> v{true};
    auto ref = v[0];
    static_assert(!std::is_same<decltype(ref), bool>::value,
                  "sanity check failed");
    auto val = Dune::autoCopy(v[0]);
    static_assert(std::is_same<decltype(val), bool>::value,
                  "vector<bool>::reference not resolved");
  }

  {
    constexpr ZeroExpr zexpr{};
    auto val = doAutoCopy(zexpr);
    static_assert(std::is_same<decltype(val), int>::value,
                  "Custom type was not resolved");

    static_assert(doAutoCopy(zexpr) == 0,
                  "Resolution is not constexpr");
  }

}
