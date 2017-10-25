#include "config.h"

#include <type_traits>
#include <utility>
#include <vector>

#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>

template<class T>
constexpr auto doResolve(T &&v)
{
  return Dune::resolveRefs(std::forward<T>(v));
}

struct ZeroProxy {};

namespace Dune {
  namespace ResolveRefsOverloads {
    constexpr auto resolveRefs(ADLTag, PriorityTag<0>, ZeroProxy)
    {
      return 0;
    }
  } // namespace ResolveRefsOverloads

  // do resolve should not pick up this overload
  constexpr auto resolveRefs(ZeroProxy) = delete;
} // namespace Dune

int main()
{

  {
    std::vector<bool> v{true};
    auto ref = v[0];
    static_assert(!std::is_same<decltype(ref), bool>::value,
                  "sanity check failed");
    auto val = Dune::resolveRefs(v[0]);
    static_assert(std::is_same<decltype(val), bool>::value,
                  "vector<bool>::reference not resolved");
  }

  {
    constexpr ZeroProxy zproxy{};
    auto val = doResolve(zproxy);
    static_assert(std::is_same<decltype(val), int>::value,
                  "Custom type was not resolved");

    static_assert(doResolve(zproxy) == 0,
                  "Resolution is not constexpr");
  }

}
