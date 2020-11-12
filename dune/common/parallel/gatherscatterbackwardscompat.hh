#ifndef DUNE_GATHERSCATTERBACKWARDSCOMPAT
#define DUNE_GATHERSCATTERBACKWARDSCOMPAT

#include <utility>
#include <dune/common/std/type_traits.hh>

namespace Dune {
  // Tools for backwards compatibility with GatherScatter interface not receiving sender/receiver ranks
  namespace Impl {
    struct Any { template <typename U> operator U( void ); };
    struct AnyRef { template <typename U> operator U&( void ); };

    template<class T, template<class...> class Op, class... Args>
    constexpr bool isValidWithArgs(Args&&...)
    { return Std::is_detected_v<Op, T, Args...>; }

    template<class T, class... Args>
    using DetectedGather = decltype(std::declval<T>().gather(std::forward<Args>(std::declval<Args>())...));

    template<class T, class... Args>
    using DetectedScatter = decltype(std::declval<T>().scatter(std::forward<Args>(std::declval<Args>())...));

    template<class T, class... Args>
    using DetectedSize = decltype(std::declval<T>().size(std::forward<Args>(std::declval<Args>())...));
  }
}
#endif
