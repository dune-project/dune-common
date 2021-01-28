// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_UNREACHABLE_HH
#define DUNE_COMMON_UNREACHABLE_HH

#include <dune/common/exceptions.hh>

#ifdef DOXYGEN

//! Mark a point in the program flow as unreachable.
/**
 * It is useful in situations where the compiler cannot deduce the unreachability of the code and
 * may help the optimizer to eliminate these code segements and additional tests.
 *
 * Examples are switch-case statements defined for all enum values. There, the default
 * case is unreachable.
 *
 * If the code is compiled without NDEBUG an exception is thrown if the unreachable code is reached.
 *
 * Example of usage:
 * \code
 * enum E { e1, e2 };
 * E e = e2;
 *
 * int x = 0;
 * switch (e)
 * {
 *   case e1: x = 1; break;
 *   case e2: x = 2; break;
 *   default: DUNE_UNREACHABLE;
 * }
 * \endcode
 */
#define DUNE_UNREACHABLE implementation_defined


//! Mark a point in the program flow as unreachable and print an error message if reached unexpectely.
/**
 * Same as \ref DUNE_UNREACHABLE but provides an additonal message argument to give details on potential
 * failure.
 *
 * Example of usage:
 * \code
 * DUNE_UNREACHABLE_MSG("Unhandled enumeration value of enum 'E'");
 * \endcode
 */
#define DUNE_UNREACHABLE_MSG implementation_defined

#else // DOXYGEN


#ifndef NDEBUG
# define DUNE_UNREACHABLE DUNE_THROW(Dune::Exception, "Code unreachable")
#else
# ifdef _MSC_VER
#   define DUNE_UNREACHABLE __assume(false)
# else
#   define DUNE_UNREACHABLE __builtin_unreachable()
# endif
#endif


#ifndef NDEBUG
# define DUNE_UNREACHABLE_MSG(msg) DUNE_THROW(Dune::Exception, msg)
#else
# define DUNE_UNREACHABLE_MSG(msg) DUNE_UNREACHABLE
#endif

#endif // DOXYGEN

#endif // DUNE_COMMON_UNREACHABLE_HH
