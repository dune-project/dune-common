// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_DEBUGSTREAM_HH
#define DUNE_DEBUGSTREAM_HH

#include <iostream>
#include <stack>

#include <dune/common/exceptions.hh>

namespace Dune {

  /*! \file

     This file implements the class DebugStream to support output in a
     variety of debug levels. Additionally, template parameters control
     if the output operation is really performed so that unused debug
     levels can be deactivated

   */

  /*! \defgroup DebugOut Debug output
     \ingroup Common

     The debug output is implemented by instaces of DebugStream which
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
     of DebugStreams so that the debug levels can be changed seperately.
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

  //! type for debug levels. Only positive values allowed
  typedef unsigned int DebugLevel;

  /*!
     value is false if current is below the threshold, true otherwise

     This is the default struct to control the activation policy of
     DebugStream and deactivates output below the threshold
   */
  template <DebugLevel current, DebugLevel threshold>
  struct greater_or_equal {
    static const bool value = greater_or_equal<current-1, threshold>::value;
  };

  template < DebugLevel threshold >
  struct greater_or_equal<0, threshold> {
    static const bool value = false;
  };

  template < DebugLevel threshold >
  struct greater_or_equal<threshold, threshold> {
    static const bool value = true;
  };

  /*! data component value is true if template parameter is non-zero

     this template is used to implement the common_bits template
   */
  template <DebugLevel x>
  struct notzero {
    static const bool value = true;
  };

  template < >
  struct notzero<0> {
    static const bool value = false;
  };

  /*! activate if current and mask have common bits switched on

     This template implements an alternative strategy to activate or
     deactivate a DebugStream. Keep in mind to number your streams as
     powers of two if using this template
   */
  template <DebugLevel current, DebugLevel mask>
  struct common_bits {
    static const bool value = notzero<current & mask>::value;
  };


  //! standard exception for the debugstream
  class DebugStreamError : public IOError {};

  class StreamWrap {
  public:
    StreamWrap(std::ostream& _out) : out(_out) { };
    std::ostream& out;
    StreamWrap *next;
  };

  //! \internal intermediate class to implement tie-operation of DebugStream
  class DebugStreamState {
    // !!! should be protected somehow but that won't be easy
  public:
    //! current output stream and link to possibly pushed old output streams
    StreamWrap* current;

    //! flag to switch output during runtime
    bool _active;

    //! are we tied to another DebugStream?
    bool _tied;

    //! how many streams are tied to this state
    unsigned int _tied_streams;
  };

  /*!
     generic class to implement debug output streams

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
    /*! create a DebugStream and set initial output stream

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
    };

    /*! create a DebugStream and directly tie to another DebugStream

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
    };

    /*! destroy stream

       if other streams still tie() to this stream an exception will be
       thrown. Otherwise the child streams would certainly break on the
       next output
     */
    ~DebugStream() {
      // untie
      if (_tied)
        tiedstate->_tied_streams--;
      else {
        // check if somebody still ties to us...
        if (_tied_streams != 0)
          DUNE_THROW(DebugStreamError,
                     "There are streams still tied to this stream!");
      };

      // remove ostream-stack
      while (current != 0) {
        StreamWrap *s = current;
        current = current->next;
        delete s;
      };
    };

    //! generic types are passed on to current output stream
    template <class T>
    DebugStream& operator<<(T data)  {
      // remove the following code if stream wasn't compiled active
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            current->out << data;
        } else {
          if (_active && tiedstate->_active)
            tiedstate->current->out << data;
        };
      };

      return *this;
    }

    //! pass on manipulators to underlying output stream
    DebugStream& operator<<(std::ostream& (*f)(std::ostream&)) {
      if (activator<thislevel, dlevel>::value) {
        if (! _tied) {
          if (_active)
            f(current->out);
        } else {
          if (_active && tiedstate->_active)
            f(tiedstate->current->out);
        };
      }

      return *this;
    };

    //! set activation flag and store old value
    void push(bool b) {
      // are we at all active?
      if (activator<thislevel,alevel>::value) {
        _actstack.push(_active);
        _active = b;
      } else {
        // stay off
        _actstack.push(false);
      };
    };

    //! restore previously set activation flag
    void pop() throw(DebugStreamError) {
      if (_actstack.empty())
        DUNE_THROW(DebugStreamError, "No previous activation setting!");

      _active = _actstack.top();
      _actstack.pop();
    };

    /*! reports if this stream will produce output

       a DebugStream that is deactivated because of its level will always
       return false, otherwise the state of the internal activation is
       returned
     */
    bool active() const {
      return activator<thislevel, dlevel>::value && _active;
    };

    //! set output to a different stream. Old stream data is stored
    void attach(std::ostream& stream) {
      if (_tied)
        DUNE_THROW(DebugStreamError, "Cannot attach to a tied stream!");

      StreamWrap* newcurr = new StreamWrap(stream);
      newcurr->next = current;
      current = newcurr;
    };

    //! detach current output stream and restore to previous stream
    void detach() throw(DebugStreamError) {
      if (current->next == 0)
        DUNE_THROW(DebugStreamError, "Cannot detach initial stream!");
      if (_tied)
        DUNE_THROW(DebugStreamError, "Cannot detach a tied stream!");

      StreamWrap* old = current;
      current = current->next;
      delete old;
    };

    void tie(DebugStreamState& to) throw(DebugStreamError) {
      if (to._tied)
        DUNE_THROW(DebugStreamError, "Cannot tie to an already tied stream!");
      if (_tied)
        DUNE_THROW(DebugStreamError, "Stream already tied: untie first!");

      _tied = true;
      tiedstate = &to;

      // tell master class
      tiedstate->_tied_streams++;
    };

    void untie() throw(DebugStreamError) {
      if(! _tied)
        DUNE_THROW(DebugStreamError, "Cannot untie, stream is not tied!");

      tiedstate->_tied_streams--;
      _tied = false;
      tiedstate = 0;
    };

  private:
    //! pointer to data of stream we're tied to
    DebugStreamState* tiedstate;

    /*! store old activation settings so that the outside code doesn't
       need to remeber */
    std::stack<bool> _actstack;
  };

}


#endif
