#ifndef DUNE_BOUNDSCHECKING_HH
#define DUNE_BOUNDSCHECKING_HH

#include <dune/common/exceptions.hh>

#ifndef DUNE_ASSERT_BOUNDS
#ifdef DUNE_CHECK_BOUNDS
#define DUNE_ASSERT_BOUNDS(cond)                            \
  do {                                                      \
    if (!(cond))                                            \
      DUNE_THROW(Dune::RangeError, "Index out of bounds."); \
  } while (false)
#else
#define DUNE_ASSERT_BOUNDS(cond)
#endif
#endif

#endif
