#ifndef DUNE_COMMON_CXX11_HH
#define DUNE_COMMON_CXX11_HH

#if HAVE_CONSTEXPR
#define DUNE_CONSTEXPR constexpr
#else // #if HAVE_CONSTEXPR
#define DUNE_CONSTEXPR
#endif // #else // #if HAVE_CONSTEXPR

#endif // #ifndef DUNE_COMMON_CXX11_HH
