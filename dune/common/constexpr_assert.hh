// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONSTEXPR_ASSERT_HH
#define DUNE_COMMON_CONSTEXPR_ASSERT_HH


// The macro DUNE_CONSTEXPR_ASSERT can be used as expression in the return
// statement of a constexpr function to have assert() and constexpr at the
// same time.
#ifdef NDEBUG
  #define DUNE_CONSTEXPR_ASSERT(X) 0
#else
  #define DUNE_CONSTEXPR_ASSERT(X) !(X) ? throw [&](){assert(!#X);return 0;}() : 0
#endif



#endif
