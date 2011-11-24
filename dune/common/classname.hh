// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CLASSNAME_HH
#define DUNE_CLASSNAME_HH

/** \file
 * \brief A free function to provide the demangled class name
 *        of a given object or type as a string
 */

#include <cstdlib>
#include <string>
#include <typeinfo>

#ifdef __GNUC__
#include <cxxabi.h>
#endif // #ifdef __GNUC__

namespace Dune {

  /** \brief Provide the demangled class name of a given object as a string */
  template <class T>
  std::string className ( T &t )
  {
    std::string className = typeid( t ).name();
#ifdef __GNUC__
    int status;
    char *demangled = abi::__cxa_demangle( className.c_str(), 0, 0, &status );
    if( demangled )
    {
      className = demangled;
      std::free( demangled );
    }
#endif // #ifdef __GNUC__
    return className;
  };

  /** \brief Provide the demangled class name of a type T as a string */
  template <class T>
  std::string className ()
  {
    std::string className = typeid( T ).name();
#ifdef __GNUC__
    int status;
    char *demangled = abi::__cxa_demangle( className.c_str(), 0, 0, &status );
    if( demangled )
    {
      className = demangled;
      std::free( demangled );
    }
#endif // #ifdef __GNUC__
    return className;
  };

} // namespace Dune

#endif  // DUNE_CLASSNAME_HH
