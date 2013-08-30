/* config.h.  Generated from config.h.cmake by CMake.  */

/* begin dune-common */

#ifdef DEPRECATED_MPI_CPPFLAGS_USED
#warning The MPI_CPPFLAGS configure substitute is deprecated. Please change
#warning your Makefile.am to use DUNEMPICPPFLAGS instead. Note that it is a
#warning good idea to change any occurance of MPI_LDFLAGS into DUNEMPILIBS and
#warning DUNEMPILDFLAGS as appropriate, since it is not possible to issue a
#warning deprecation warning in that case.
#endif

/* Define to the version of dune-common */
#define DUNE_COMMON_VERSION "${DUNE_COMMON_VERSION}"

/* Define to the major version of dune-common */
#define DUNE_COMMON_VERSION_MAJOR ${DUNE_COMMON_VERSION_MAJOR}

/* Define to the minor version of dune-common */
#define DUNE_COMMON_VERSION_MINOR ${DUNE_COMMON_VERSION_MINOR}

/* Define to the revision of dune-common */
#define DUNE_COMMON_VERSION_REVISION ${DUNE_COMMON_VERSION_REVISION}

/* Standard debug streams with a level below will collapse to doing nothing */
#define DUNE_MINIMAL_DEBUG_LEVEL ${DUNE_MINIMAL_DEBUG_LEVEL}

/* does the compiler support __attribute__((deprecated))? */
#cmakedefine HAS_ATTRIBUTE_DEPRECATED 1

/* does the compiler support __attribute__((deprecated("message"))? */
#cmakedefine HAS_ATTRIBUTE_DEPRECATED_MSG 1

/* does the compiler support __attribute__((unused))? */
#cmakedefine HAS_ATTRIBUTE_UNUSED 1

/* Define to 1 if the <array> C++11 is available and support array::fill */
#cmakedefine HAVE_ARRAY 1

/* Define if you have a BLAS library. */
#cmakedefine HAVE_BLAS 1

/* Define to ENABLE_BOOST if the Boost library is available */
#cmakedefine HAVE_DUNE_BOOST ENABLE_BOOST

/* Define to 1 if you have <boost/make_shared.hpp>. */
#cmakedefine HAVE_BOOST_MAKE_SHARED_HPP 1

/* Define to 1 if you have the <boost/shared_ptr.hpp> header file. */
#cmakedefine HAVE_BOOST_SHARED_PTR_HPP 1

/* does the compiler support abi::__cxa_demangle */
#cmakedefine HAVE_CXA_DEMANGLE 1

/* Define if you have LAPACK library. */
#cmakedefine HAVE_LAPACK 1

/* Define to 1 if SHARED_PTR_NAMESPACE::make_shared is usable. */
#cmakedefine HAVE_MAKE_SHARED 1

/* Define to 1 if you have the <malloc.h> header file. */
// Not used! #cmakedefine01 HAVE_MALLOC_H

/* Define if you have the MPI library. This is only true if MPI was found by
   configure _and_ if the application uses the DUNEMPICPPFLAGS (or the
   deprecated MPI_CPPFLAGS) */
#cmakedefine HAVE_MPI ENABLE_MPI

/* Define if you have the GNU GMP library. The value should be ENABLE_GMP
   to facilitate activating and deactivating GMP using compile flags. */
#cmakedefine HAVE_GMP ENABLE_GMP

/* Define to 1 if you have the symbol mprotect. */
#cmakedefine HAVE_MPROTECT 1

/* Define to 1 if nullptr is supported */
#cmakedefine HAVE_NULLPTR 1

/* Define to 1 if static_assert is supported */
#cmakedefine HAVE_STATIC_ASSERT 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have <sys/mman.h>. */
#cmakedefine HAVE_SYS_MMAN_H 1

/* Define to 1 if the std::tr1::hash template from TR1 is available. */
#cmakedefine HAVE_TR1_HASH 1

/* Define to 1 if you have the <tr1/tuple> header file. */
#cmakedefine HAVE_TR1_TUPLE 1

/* Define to 1 if you have the <tr1/type_traits> header file. */
#cmakedefine HAVE_TR1_TYPE_TRAITS 1

/* Define to 1 if std::integral_constant< T, v > is supported
 * and casts into T
 */
#cmakedefine HAVE_INTEGRAL_CONSTANT 1

/* Define to 1 if the std::hash template from C++11 is available. */
#cmakedefine HAVE_STD_HASH 1

/* Define to 1 if you have the <tuple> header file. */
#cmakedefine HAVE_TUPLE 1

/* Define to 1 if you have the <type_traits> header file. */
#cmakedefine HAVE_TYPE_TRAITS 1

/* Define to 1 if you have the <type_traits> header file. */
#cmakedefine HAVE_STD_CONDITIONAL 1

/* Define to 1 if the MPI2 Standard is supported */
#cmakedefine MPI_2 1

/* begin private */

/* Name of package */
#define PACKAGE "dune-common"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@DUNE_MAINTAINER@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@DUNE_MOD_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@DUNE_MOD_NAME@ @DUNE_MOD_VERSION@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@DUNE_MOD_NAME@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@DUNE_MOD_URL@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@DUNE_MOD_VERSION@"

/* Version number of package */
#define VERSION "@DUNE_MOD_VERSION@"

/* end private */

/* The header in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_HEADER ${SHARED_PTR_HEADER}

/* The namespace in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_NAMESPACE ${SHARED_PTR_NAMESPACE}

/* Define to 1 if variadic templates are supported */
#cmakedefine HAVE_VARIADIC_TEMPLATES 1

/* Define to 1 if SFINAE on variadic template constructors is fully supported */
#cmakedefine HAVE_VARIADIC_CONSTRUCTOR_SFINAE 1

/* Define to 1 if rvalue references are supported */
#cmakedefine HAVE_RVALUE_REFERENCES 1

/* Define to 1 if initializer list is supported */
#cmakedefine HAVE_INITIALIZER_LIST 1

/* Include always useful headers */
#include <dune/common/deprecated.hh>
#include <dune/common/unused.hh>
#include "FC.h"
#define FC_FUNC FC_GLOBAL_

/* end dune-common */
