// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/*

   Declaration of standard Dune-library streams

 */

#ifndef DUNE_COMMON_STDSTREAMS_HH
#define DUNE_COMMON_STDSTREAMS_HH

#include "debugstream.hh"

namespace Dune {

  /*!
     \addtogroup DebugOut
     @{

     standard debug streams with level below MINIMAL_DEBUG_LEVEL will
     collapse to doing nothing if output is requested.

     For a Dune-Release this should be set to at least 4 so that only
     important messages are active. Dune-developers may adapt this
     setting to their debugging needs locally

     Keep in mind that libdune has to be recompiled if this value is changed!



     The singleton instances of the available debug streams can befound in
     the \ref DebugOut "Standard Debug Streams" module
   */

  /*! \file

     The standard debug streams are compiled into libdune to exist
     globally. This file declares the stream types and the global debug
     level.

   */
  /*! @} */
  /*!
     \defgroup StdStreams Standard Debug Streams
     \ingroup DebugOut
     @{

     Dune defines several standard output streams for the library
     routines.

     Applications may control the standard streams via the attach/detach,
     push/pop interface but should define an independent set of streams (see \ref DebugAppl )

   */

  /**
   * @brief The default minimum debug level.
   *
   * If the  level of a stream is bigger than this value
   * it will be activated.
   */
  static const DebugLevel MINIMAL_DEBUG_LEVEL = 4;

  /** @brief Type of very verbose debug stream. */
  typedef DebugStream<1, MINIMAL_DEBUG_LEVEL> DVVerbType;

  /*!
     \brief stream for very verbose output.

     Information on the lowest
     level. This is expected to report insane amounts of
     information. Use of the activation-flag to only generate output
     near the problem is recommended.
   */
  extern DVVerbType dvverb;

  /** @brief Type of more verbose debug stream.*/
  typedef DebugStream<2, MINIMAL_DEBUG_LEVEL> DVerbType;
  /** @brief Singleton of more verbose debug stream. */
  extern DVerbType dverb;

  /** @brief Type of debug stream with info level.*/
  typedef DebugStream<3, MINIMAL_DEBUG_LEVEL> DInfoType;

  /**
      @brief Stream for informative output.

      Summary infos on what a module
      does, runtimes, etc.
   */
  extern DInfoType dinfo;

  /** @brief Type of debug stream with warn level.*/
  typedef DebugStream<4, MINIMAL_DEBUG_LEVEL> DWarnType;

  /** @brief Stream for warnings indicating problems. */
  extern DWarnType dwarn;

  /** @brief Type of debug stream for fatal errors.*/
  typedef DebugStream<5, MINIMAL_DEBUG_LEVEL> DGraveType;

  /**  @brief Stream for warnings indicating fatal errors.*/
  extern DGraveType dgrave;

  /** @brief The type of the stream used for error messaged. */
  typedef DebugStream<1> DErrType;

  /*!
     @brief Stream for error messages.

     Only packages integrating Dune
     completely will redirect it. The output of derr is independant on
     the debug-level, only the activation-flag is checked.
   */
  extern DErrType derr;

  //! }@
}

#endif
