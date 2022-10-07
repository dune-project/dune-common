// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

/**
   \file
   \brief Standard Dune debug streams

   The standard debug streams are compiled into libdune to exist
   globally. This file declares the stream types and the global debug
   level.
 */

#ifndef DUNE_COMMON_STDSTREAMS_HH
#define DUNE_COMMON_STDSTREAMS_HH

#include "debugstream.hh"

namespace Dune {

  /**
      \addtogroup DebugOut
      @{

      standard debug streams with level below MINIMAL_DEBUG_LEVEL will
      collapse to doing nothing if output is requested.

      MINIMAL_DEBUG_LEVEL is set to DUNE_MINIMAL_DEBUG_LEVEL, which is
      defined in config.h and can be changed by the configure option
      @code --with-minimal-debug-level=[grave|warn|info|verb|vverb] @endcode

      For a Dune-Release this should be set to at least 4 so that only
      important messages are active. Dune-developers may adapt this
      setting to their debugging needs locally

      Keep in mind that libdune has to be recompiled if this value is changed!



      The singleton instances of the available debug streams can be found in
      the \ref DebugOut "Standard Debug Streams" module

      @}
   */

  /**
     \defgroup StdStreams Standard Debug Streams
     \ingroup DebugOut
     @{

     Dune defines several standard output streams for the library
     routines.

     Applications may control the standard streams via the attach/detach,
     push/pop interface but should define an independent set of streams (see \ref DebugAppl )

   */

  /**
      @brief The default minimum debug level.

      If the  level of a stream is bigger than this value
      it will be activated.
   */
#ifndef DUNE_MINIMAL_DEBUG_LEVEL
#define DUNE_MINIMAL_DEBUG_LEVEL 4
#endif
  static const DebugLevel MINIMAL_DEBUG_LEVEL = DUNE_MINIMAL_DEBUG_LEVEL;

  /**
      @brief The level of the very verbose debug stream.
      @see dvverb
   */
  static const DebugLevel VERY_VERBOSE_DEBUG_LEVEL = 1;

  /**
      @brief Type of very verbose debug stream.
      @see dvverb
   */
  typedef DebugStream<VERY_VERBOSE_DEBUG_LEVEL, MINIMAL_DEBUG_LEVEL> DVVerbType;

  /**
      \brief stream for very verbose output.

      \code
     #include <dune/common/stdstreams.hh>
      \endcode

      Information on the lowest
      level. This is expected to report insane amounts of
      information. Use of the activation-flag to only generate output
      near the problem is recommended.
   */
  extern DVVerbType dvverb;

  /**
      @brief The level of the verbose debug stream.
      @see dvverb
   */
  static const DebugLevel VERBOSE_DEBUG_LEVEL = 2;

  /**
      @brief Type of more verbose debug stream.
      @see dverb
   */
  typedef DebugStream<VERBOSE_DEBUG_LEVEL, MINIMAL_DEBUG_LEVEL> DVerbType;

  /**
     @brief Singleton of verbose debug stream.

     \code
     #include <dune/common/stdstreams.hh>
     \endcode
   */
  extern DVerbType dverb;

  /**
      @brief The level of the informative debug stream.
      @see dinfo
   */
  static const DebugLevel INFO_DEBUG_LEVEL = 3;

  /**
      @brief Type of debug stream with info level.
      @see dinfo
   */
  typedef DebugStream<INFO_DEBUG_LEVEL, MINIMAL_DEBUG_LEVEL> DInfoType;

  /**
      @brief Stream for informative output.

      \code
     #include <dune/common/stdstreams.hh>
      \endcode

      Summary infos on what a module
      does, runtimes, etc.
   */
  extern DInfoType dinfo;

  /**
      @brief The level of the debug stream for warnings.
      @see dwarn
   */
  static const DebugLevel WARN_DEBUG_LEVEL = 4;

  /**
      @brief Type of debug stream with warn level.
      @see dwarn
   */
  typedef DebugStream<WARN_DEBUG_LEVEL, MINIMAL_DEBUG_LEVEL> DWarnType;

  /**
     @brief Stream for warnings indicating problems.

     \code
     #include <dune/common/stdstreams.hh>
     \endcode
   */
  extern DWarnType dwarn;

  /**
      @brief The level of the debug stream for fatal errors.
      @see dgrave
   */
  static const DebugLevel GRAVE_DEBUG_LEVEL = 5;

  /** @brief Type of debug stream for fatal errors.*/
  typedef DebugStream<GRAVE_DEBUG_LEVEL, MINIMAL_DEBUG_LEVEL> DGraveType;

  /**
     @brief Stream for warnings indicating fatal errors.

     \code
     #include <dune/common/stdstreams.hh>
     \endcode
   */
  extern DGraveType dgrave;

  /** @brief The type of the stream used for error messages. */
  typedef DebugStream<1> DErrType;

  /**
     @brief Stream for error messages.

     \code
     #include <dune/common/stdstreams.hh>
     \endcode

     Only packages integrating Dune
     completely will redirect it. The output of derr is independent of
     the debug-level, only the activation-flag is checked.
   */
  extern DErrType derr;

  /** }@ */
}

#endif
