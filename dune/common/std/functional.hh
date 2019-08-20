// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set ts=8 sw=2 et sts=2:
#ifndef DUNE_COMMON_STD_FUNCTIONAL_HH
#define DUNE_COMMON_STD_FUNCTIONAL_HH

#include <functional>

namespace Dune
{

  namespace Std
  {
#if DUNE_HAVE_CXX_STD_IDENTITY
    using std::identity;
#else //DUNE_HAVE_CXX_STD_IDENTITY
    struct identity {
      template<class T>
      constexpr T&& operator()(T&& t ) const noexcept {return std::forward<T>(t);}
    };
#endif //DUNE_HAVE_CXX_STD_IDENTITY
  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_FUNCTIONAL_HH
