#ifndef DUNE_COMMON_STD_UTILITY_HH
#define DUNE_COMMON_STD_UTILITY_HH

#include <cstddef>

#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>

namespace Dune
{

  namespace Std
  {


    using std::integer_sequence;
    using std::index_sequence;
    using std::make_integer_sequence;
    using std::make_index_sequence;

    /**
     * \brief Create index_sequence from 0 to sizeof...(T)-1
     *
     * This should do the same as std::index_sequence_for.
     * But due to a bug in the sizeof... operator this
     * may produce wrong results with clang<3.8.
     *
     * As a workaround we provide our own implementation
     * that avoids this bug even if the std:: version
     * exists.
     *
     * This implemenation can be dropped, once we require
     * a minimum clang version that has this bug fixed (i.e. >=3.8).
     */
    template<typename... T>
    using index_sequence_for = make_index_sequence<typename Dune::SizeOf<T...>{}>;



  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_UTILITY_HH
