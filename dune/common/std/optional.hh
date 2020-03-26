#ifndef DUNE_COMMON_STD_OPTIONAL_HH
#define DUNE_COMMON_STD_OPTIONAL_HH

#include <cassert>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <optional>

#warning dune/common/std/optional.hh is deprecated and will be removed after Dune 2.8.\
 Include <optional> instead

namespace Dune
{

  namespace Std
  {
    // In case of C++ standard >= 17 we forward optionals into our namespace
    template< class T >
    using optional = std::optional< T >;

    using nullopt_t = std::nullopt_t;
    using in_place_t = std::in_place_t;

    namespace
    {
      const std::nullopt_t nullopt = std::nullopt;
      const std::in_place_t in_place =  std::in_place;
    } // anonymous namespace

    using bad_optional_access = std::bad_optional_access;

  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_OPTIONAL_HH
