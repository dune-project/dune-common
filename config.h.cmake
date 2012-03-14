#define DUNE_COMMON_FIELDVECTOR_SIZE_IS_METHOD 1

#cmakedefine01 HAVE_MPI
#cmakedefine01 HAVE_MALLOC_H
#cmakedefine01 HAVE_BOOST

/* If this is set, the member 'size' of FieldVector is a method rather than an
   enum */
#define DUNE_COMMON_FIELDVECTOR_SIZE_IS_METHOD 1

/* Define to 1 if the <array> C++11 is available and support array::fill */
#cmakedefine01 HAVE_ARRAY

/* Define to 1 if you have the <memory> header file. */
#cmakedefine01 HAVE_MEMORY

/* The namespace in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_NAMESPACE ${SHARED_PTR_NAMESPACE}

/* The header in which SHARED_PTR can be found */
#cmakedefine SHARED_PTR_HEADER ${SHARED_PTR_HEADER}

/* Define to 1 if SHARED_PTR_NAMESPACE::make_shared is usable */
#cmakedefine01 HAVE_MAKE_SHARED

/* Define to 1 if you have <boost/make_shared.hpp> */
#cmakedefine01 HAVE_BOOST_MAKE_SHARED_HPP

#cmakedefine01 HAVE_NULLPTR
#cmakedefine01 HAVE_STATIC_ASSERT
#cmakedefine01 HAVE_VARIADIC_TEMPLATES
#cmakedefine01 HAVE_VARIADIC_CONSTRUCTOR_SFINAE
#cmakedefine01 HAVE_RVALUE_REFERENCES

/* Include allways usefull headers */
#include <dune/common/deprecated.hh>
#include <dune/common/unused.hh>
