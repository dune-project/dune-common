// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_DEBUGSTREAM_HH
#define DUNE_DEBUGSTREAM_HH

/** \file
 * \brief Defines several output streams for messages of different importance
 */

#include <iostream>
#include <stack>

#include <dune/common/exceptions.hh>

namespace Dune {

  /*! \defgroup DebugOut Debug output
     \ingroup Common

     The debug output is implemented by instances of DebugStream which
     provides the following features:

     - output-syntax in the standard ostream-notation
     - output can be totally deactivated depending on template parameters
     - streams with active output can be deactivated during runtime
     - redirecting to std::ostream or other DebugStream s during runtime
     - stack oriented state

     The Dune-components should use the streams explained in \ref StdStreams
     for output so that applications may redirect the output globally.

     Changes in runtime are provided by three sets of methods:

     - push()/pop() sets new activation flag or restore old setting
     - attach()/detach() redirects output to a different std::ostream or restore old stream
     - tie()/untie() redirects output through another DebugStream. If the state of the master stream changes (activation or output-stream) it is changed in the tied stream as well

     The first methods implement a full stack whereas tie() is a bit
     different: though a tied stream may be (de)activated via
     push()/pop() you cannot attach() or detach() an output. You'll need
     to change the master stream instead.

     \section DebugAppl Applications

     Applications using the Dune-library should create an independent set
     of DebugStreams so that the debug levels can be changed separately.
     Example:

     \code
     static const Dune::DebugLevel APPL_MINLEVEL = 3;

     Dune::DebugStream<1, APPL_MINLEVEL> myverbose;
     Dune::DebugStream<2, APPL_MINLEVEL> myinfo;
     Dune::DebugStream<3, APPL_MINLEVEL> mywarn;
     \endcode

     This code creates three streams of which only the last one really
     creates output. The output-routines of the other streams vanish in
     optimized executables.

     You can use the common_bits-Template to switch to a policy using bitflags:

     \code
     enum { APPL_CORE = 1, APPL_IO = 2, APPL_GRAPHICS = 4};

     static const Dune::DebugLevel APPL_DEBUG_MASK = APPL_CORE | APPL_GRAPHICS;
     static const Dune::DebugLevel APPL_ACTIVE_MASK = 0xff;

     Dune::DebugStream<APPL_CORE, APPL_DEBUG_MASK, APPL_ACTIVE_MASK, Dune::common_bits> coreout;
     Dune::DebugStream<APPL_IO, APPL_DEBUG_MASK, APPL_ACTIVE_MASK, Dune::common_bits> ioout;
     Dune::DebugStream<APPL_GRAPHICS, APPL_DEBUG_MASK, APPL_ACTIVE_MASK, Dune::common_bits> graphout;
     \endcode

     Applications that wish to redirect the \ref StdStreams through their
     private streams may use the tie()-mechanism:

     \code
     // initialize streams like above

     Dune::dwarn.tie(coreout);

     // ... Dune-output to dwarn will be directed through coreout ...

     Dune::dwarn.untie();
     \endcode

     Keep in mind to untie() a stream before the tied stream is destructed.

     An alternative is to attach() an output stream defined by the application:

     \code
     std::ofstream mylog("application.log");

     Dune::dwarn.attach(mylog);
     \endcode
   */
  /**
     \addtogroup DebugOut
     \{
   */
  /*! \file

     This file implements the class DebugStream to support output in a
     variety of debug levels. Additionally, template parameters control
     if the output operation is really performed so that unused debug
     levels can be deactivated

   */


  /*! \brief Type for debug levels.

     Only positive values allowed
   */
  typedef unsigned int DebugLevel;

  /*!

     \brief Greater or equal template test.

     value is false if current is below the threshold, true otherwise

     This is the default struct to control the activation policy of
     DebugStream and deactivates output below the threshold
   */
  template <DebugLevel current, DebugLevel threshold>
  struct greater_or_equal {
    constexpr static bool value = (current >= threshold);
  };


  /*! \brief activate if current and mask have common bits switched on.

     This template implements an alternative strategy to activate or
     deactivate a DebugStream. Keep in mind to number your streams as
     powers of two if using this template
   */
  template <DebugLevel current, DebugLevel mask>
  struct common_bits {
    constexpr static bool value = ((current & mask) != 0);
  };


  //! \brief standard exception for the debugstream
  class DebugStreamError : public IOError {};

  class StreamWrap {
  public:
    StreamWrap(std::ostream& _out) : out(_out) { }
    std::ostream& out;
    StreamWrap *next;
  };

  //! \brief Intermediate class to implement tie-operation of DebugStream
  class DebugStreamState {
    // !!! should be protected somehow but that won't be easy
  public:
    //! \brief current output stream and link to possibly pushed old output streams
    StreamWrap* current;

    //! \brief flag to switch output during runtime
    bool _active;

    //! \brief are we tied to another DebugStream?
    bool _tied;

    //! \brief how many streams are tied to this state
    unsigned int _tied_streams;
  };

  /*!
     \brief Generic class to implement debug output streams

     The main function of a DebugStream is to provide output in a
     standard ostream fashion that is fully deactivated if the level of
     the stream does not meet the current requirements. More information in \ref DebugOut

     \param thislevel this level
     \param dlevel level needed for any output to happen
     \param alevel level needed to switch activation flag on
     \param activator template describing the activation policy

     \todo Fix visibility of internal data
   */
  template <DebugLevel thislevel = 1,
      DebugLevel dlevel = 1,
      DebugLevel alevel = 1,
      template<DebugLevel, DebugLevel> class activator = greater_or_equal>
  class DebugStream : public DebugStreamState {
  public:
    /*! \brief Create a DebugStream and set initial output stream

       during runtime another stream can be attach()ed, however the
       initial stream may not be detach()ed.
     */
    DebugStream(std::ostream& out = std::cerr) {
      // start a new list of streams
      current = new StreamWrap(out);
      current->next = 0;

      // check if we are above the default activation level
      _active = activator<thislevel,alevel>::value;

      // we're not tied to another DebugStream
      _tied = false;

      // no child streams yet
      _tied_streams = 0;
    }

    /*! \brief Create a DebugStream and directly tie to another DebugStream

       The fallback is used if a DebugStream constructed via this method
       is untie()ed later. Otherwise the stream would be broken afterwards.
     */
    DebugStream (DebugStreamState& master,
                 std::ostream& fallback = std::cerr)
    {
      // start a new list of streams
      current = new StreamWrap(fallback);
      current->next = 0;

      // check if we are above the default activation level
      _active = activator<thislevel,alevel>::value;
      _tied_streams = 0;

      // tie to the provided stream
      _tied = true;
      tiedstate = &master;
      tiedstate->_tied_streams++;
    }

    /*! \brief Destroy stream.

       If other streams still tie() to this stream the destructor
       will call std::terminate() because you can hardly recover
       from this problem and the child streams would certainly break on the
       next output.
     */
    ~DebugStream()
    {
      // untie
      if (_tied)
        tiedstate->_tied_streams--;
      else {
        // check if somebody still ties to us...
        if (_tied_streams != 0)
        {
          std::cerr << "DebugStream destructor is called while other streams are still tied to it. Terminating!" << std::endl;
          std::terminate();
        }
      }

      // remove ostream-stack
      while (current != 0) {
        StreamWrap *s = current;
        current = current->next;
        delete s;
      }
    }

    //! \brief Generic types are passed on to current output stream
    template <class T>
    DebugStream& operator<<(const T data) {
      // remove the following code if stream wasn't compiled active
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            current->out << data;
        } else {
          if (_active && tiedstate->_active)
            tiedstate->current->out << data;
        }
      }

      return *this;
    }

    /*! \brief explicit specialization so that enums can be printed

       Operators for built-in types follow special
       rules (§11.2.3) so that enums won't fit into the generic
       method above. With an existing operator<< for int however
       the enum will be automatically casted.
     */
    DebugStream& operator<<(const int data) {
      // remove the following code if stream wasn't compiled active
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            current->out << data;
        } else {
          if (_active && tiedstate->_active)
            tiedstate->current->out << data;
        }
      }

      return *this;
    }

    //! \brief pass on manipulators to underlying output stream
    DebugStream& operator<<(std::ostream& (*f)(std::ostream&)) {
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            f(current->out);
        } else {
          if (_active && tiedstate->_active)
            f(tiedstate->current->out);
        }
      }

      return *this;
    }

    //! \brief pass on flush to underlying output stream
    DebugStream& flush() {
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            current->out.flush();
        } else {
          if (_active && tiedstate->_active)
            tiedstate->current->out.flush();
        }
      }

      return *this;
    }

    //! \brief set activation flag and store old value
    void push(bool b) {
      // are we at all active?
      if (activator<thislevel,alevel>::value) {
        _actstack.push(_active);
        _active = b;
      } else {
        // stay off
        _actstack.push(false);
      }
    }

    /*! \brief restore previously set activation flag
     * \throws DebugStreamError
     */
    void pop() {
      if (_actstack.empty())
        DUNE_THROW(DebugStreamError, "No previous activation setting!");

      _active = _actstack.top();
      _actstack.pop();
    }

    /*! \brief reports if this stream will produce output

       a DebugStream that is deactivated because of its level will always
       return false, otherwise the state of the internal activation is
       returned
     */
    bool active() const {
      return activator<thislevel, dlevel>::value && _active;
    }

    /*! \brief set output to a different stream.

       Old stream data is stored
     */
    void attach(std::ostream& stream) {
      if (_tied)
        DUNE_THROW(DebugStreamError, "Cannot attach to a tied stream!");

      StreamWrap* newcurr = new StreamWrap(stream);
      newcurr->next = current;
      current = newcurr;
    }

    /*! \brief detach current output stream and restore to previous stream
     * \throws DebugStreamError
     */
    void detach() {
      if (current->next == 0)
        DUNE_THROW(DebugStreamError, "Cannot detach initial stream!");
      if (_tied)
        DUNE_THROW(DebugStreamError, "Cannot detach a tied stream!");

      StreamWrap* old = current;
      current = current->next;
      delete old;
    }

    /*! \brief Tie a stream to this one.
     * \throws DebugStreamError
     */
    void tie(DebugStreamState& to) {
      if (to._tied)
        DUNE_THROW(DebugStreamError, "Cannot tie to an already tied stream!");
      if (_tied)
        DUNE_THROW(DebugStreamError, "Stream already tied: untie first!");

      _tied = true;
      tiedstate = &to;

      // tell master class
      tiedstate->_tied_streams++;
    }

    /*! \brief Untie stream
     * \throws DebugStreamError
     */
    void untie() {
      if(! _tied)
        DUNE_THROW(DebugStreamError, "Cannot untie, stream is not tied!");

      tiedstate->_tied_streams--;
      _tied = false;
      tiedstate = 0;
    }

  private:
    //! \brief pointer to data of stream we're tied to
    DebugStreamState* tiedstate;

    /*! \brief Activation state history.

       store old activation settings so that the outside code doesn't
       need to remember */
    std::stack<bool> _actstack;
  };

  /** /} */
}


#endif
