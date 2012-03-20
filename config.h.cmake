/* config.h.  Generated from config.h.cmake by CMake.  */

#ifdef DEPRECATED_MPI_CPPFLAGS_USED
#warning The MPI_CPPFLAGS configure substitute is deprecated. Please change
#warning your Makefile.am to use DUNEMPICPPFLAGS instead. Note that it is a
#warning good idea to change any occurance of MPI_LDFLAGS into DUNEMPILIBS and
#warning DUNEMPILDFLAGS as appropriate, since it is not possible to issue a
#warning deprecation warning in that case.
#endif

/* If this is set, the member 'size' of FieldVector is a method rather than an
   enum */
#define DUNE_COMMON_FIELDVECTOR_SIZE_IS_METHOD 1

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
// Still missing

/* Define to 1 if you have <boost/make_shared.hpp> */
#cmakedefine HAVE_BOOST_MAKE_SHARED_HPP 1

/* Define to 1 if you have the <boost/shared_ptr.hpp> header file. */
#cmakedefine HAVE_BOOST_SHARED_PTR_HPP 1

/* Define if you have LAPACK library. */

/* Define to 1 if SHARED_PTR_NAMESPACE::make_shared is usable */
#cmakedefine HAVE_MAKE_SHARED 1

/* Define to 1 if you have the <malloc.h> header file. */
// Not used! #cmakedefine01 HAVE_MALLOC_H

#cmakedefine01 HAVE_MPI
#cmakedefine01 HAVE_BOOST


/* Define to 1 if you have the <memory> header file. */
#cmakedefine01 HAVE_MEMORY

/* The namespace in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_NAMESPACE ${SHARED_PTR_NAMESPACE}

/* The header in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_HEADER ${SHARED_PTR_HEADER}


/* Define to 1 if nullptr is supported */
#cmakedefine01 HAVE_NULLPTR

/* Define to 1 if static_assert is supported */
#cmakedefine01 HAVE_STATIC_ASSERT

/* Define to 1 if variadic templates are supported */
#cmakedefine01 HAVE_VARIADIC_TEMPLATES

/* Define to 1 if SFINAE on variadic template constructors is fully supported */
#cmakedefine01 HAVE_VARIADIC_CONSTRUCTOR_SFINAE

/* Define to 1 if rvalue references are supported */
#cmakedefine01 HAVE_RVALUE_REFERENCES

/* Include allways usefull headers */
#include <dune/common/deprecated.hh>
#include <dune/common/unused.hh>
