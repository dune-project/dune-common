// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
/** @file
   @author Robert Kloefkorn
   @brief Provides check for implementation of interface methods when using
   static polymorphism, i.e. the Barton-Nackman trick. This is purely for
   debugging purposes. To check the correct implementation of interface methods
   (and pick up possible infinite loops) NDEBUG must be undefined and
   DUNE_INTERFACECHECK has to be defined.

   Use by invoking CHECK_INTERFACE_IMPLEMENTATION(asImp().methodToCheck())
   and for
   template methods double (CHECK_INTERFACE_IMPLEMENTATION((asImp().template methodToCheck<param> ())).
   If either NDEBUG is defined or
   DUNE_INTERFACECHECK is undefined the CHECK_INTERFACE_IMPLEMENTATION macro is empty.

   Note: adding the interface check to a method will cause the implementation of the
   method to be called twice, so before use make sure
   that this will not cause problems e.g. if internal counters are updated.
 **/

//- Dune includes
#include <dune/common/exceptions.hh>

#ifdef CHECK_INTERFACE_IMPLEMENTATION
#undef CHECK_INTERFACE_IMPLEMENTATION
#endif
#ifdef CHECK_AND_CALL_INTERFACE_IMPLEMENTATION
#undef CHECK_AND_CALL_INTERFACE_IMPLEMENTATION
#endif

#if defined NDEBUG || !defined DUNE_INTERFACECHECK
#define CHECK_INTERFACE_IMPLEMENTATION(dummy)
#else
#define CHECK_INTERFACE_IMPLEMENTATION(__interface_method_to_call__) \
  {\
    static bool call = false; \
    if( call == true ) \
      DUNE_THROW(NotImplemented,"Interface method not implemented!");\
    call = true; \
    try { \
      (__interface_method_to_call__); \
      call = false; \
    } \
    catch ( ... ) \
    { \
      call = false; \
      throw; \
    } \
  }
#endif

/** The macro CHECK_AND_CALL_INTERFACE_IMPLEMENTATION throws an exception,
   if the interface method is not implemented and just calls the method
   otherwise. If NDEBUG is defined no
   checking is done and the method is just called.
 */
#if defined NDEBUG || !defined DUNE_INTERFACECHECK
#define CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(__interface_method_to_call__) \
  (__interface_method_to_call__)
#else
#define CHECK_AND_CALL_INTERFACE_IMPLEMENTATION(__interface_method_to_call__) \
  CHECK_INTERFACE_IMPLEMENTATION(__interface_method_to_call__)
#endif
