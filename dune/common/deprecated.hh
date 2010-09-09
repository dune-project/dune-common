// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEPRECATED_HH
#define DUNE_DEPRECATED_HH

//! @addtogroup Common
//! @{
#if defined(DOXYGEN) or not defined(DUNE_DEPRECATED)
//! Mark some entity as deprecated
/**
 * \code
 *#include <dune/common/deprecated.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark functions,
 * typedefs, enums and other stuff deprecated.  If something is marked
 * deprecated, users are advised to migrate to the new interface, since it
 * will probably be removed in the next release of Dune.
 *
 * DUNE_DEPRECATED currently works with g++ only.  For other compilers it will
 * be defined empty.  This way the user will not get any deprecation warning,
 * but at least his code still compiles (well, until the next Dune release,
 * that is).
 *
 * Here are some examples how to mark different stuff deprecated:
 *  - Classes
 *    \code
   class DUNE_DEPRECATED Class {};
   class Class {} DUNE_DEPRECATED;
 *    \endcode
 *    This does not seem to work for template classes.  If the class template
 *    has some essential member, maybe you can get away by marking that as
 *    deprecated instead.
 *  - Member functions
 *    \code
   template<typename T> struct Class {
   void frob() DUNE_DEPRECATED {}
   };
   template<typename T> struct Class {
   void DUNE_DEPRECATED frob() {}
   };
   template<typename T> struct Class {
   DUNE_DEPRECATED void frob() {}
   };
 *    \endcode
 */
#define DUNE_DEPRECATED
#endif
//! @}

#endif
