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
/**
   If `DUNE_CHECK_BOUNDS` is defined: check if a condition holds;
   otherwise, do nothing. Meant to be used for conditions that assure
   writes and reads do not occur outside of memory limits or
   pre-defined patterns and related conditions.
*/
#define DUNE_ASSERT_BOUNDS(cond)
#endif
#endif

#endif
