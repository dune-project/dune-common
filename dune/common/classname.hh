// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CLASSNAME_HH
#define DUNE_CLASSNAME_HH

/** \file
 * \brief A free function to provide the demangled class name
 *        of a given object as a string
 */

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace Dune {

  /** \brief Provide the demangled class name of a given object as a string */
  template <class T>
  std::string className(T &t)
  {
#ifdef __GNUC__
    int status;
    return abi::__cxa_demangle(typeid(t).name(),0,0,&status);
#else
    return typeid(t).name();
#endif
  };

}
#endif  // DUNE_CLASSNAME_HH
