// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_ASSERTANDRETURN_HH
#define DUNE_COMMON_ASSERTANDRETURN_HH

#include <cassert>

// The macro DUNE_ASSERT_AND_RETURN can be used as expression in the return
// statement of a constexpr function to have assert() and constexpr at the
// same time. It first uses assert for the condition given by the first argument
// and then returns the value of the second argument.
#ifdef NDEBUG
  #define DUNE_ASSERT_AND_RETURN(C,X) X
#else
  #define DUNE_ASSERT_AND_RETURN(C,X) (!(C) ? throw [&](){assert(!#C);return 0;}() : 0), X
#endif



#endif // DUNE_COMMON_ASSERTANDRETURN_HH
