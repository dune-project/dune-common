// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_DEBUGSTREAM_HH
#define DUNE_DEBUGSTREAM_HH

#include <iostream>
#include <stack>

#include "exceptions.hh"

namespace Dune {

  /*! \file debugstream.hh */

  /*! \defgroup DebugOut Debug output

   */

  //! type for debug levels. Only positive values allowed
  typedef unsigned int DebugLevel;

  //! a structure template to act as an activator for the debugstream
  template <DebugLevel current, DebugLevel max>
  struct debug_activator {
    static const bool value = debug_activator<current-1, max>::value;
  };

  template < DebugLevel max >
  struct debug_activator<0, max> {
    static const bool value = false;
  };

  template < DebugLevel max >
  struct debug_activator<max, max> {
    static const bool value = true;
  };


  //! standard exception for the debugstream
  class DebugStreamError : public IOError {};

  /* Default: all instances active, normal activator*/

  template <DebugLevel thislevel,
      DebugLevel dlevel = 1,
      DebugLevel alevel = 1,
      template<DebugLevel, DebugLevel> class activator = debug_activator>
  class DebugStream {
  public:
    /*! create a DebugStream and set initial output stream to connect
       to. Later on a different stream may be set via attach() */
    DebugStream(std::ostream& out) {
      // start a new list of streams
      current = new streamwrap(out);
      current->next = 0;

      // check if we are above the default activation level
      if (thislevel >= alevel)
        _active = true;
      else
        _active = false;
    };

    ~DebugStream() {
      // remove ostream-stack
      while (current != 0) {
        streamwrap *s = current;
        current = current->next;
        delete s;
      };
    };

    //! generic types are passed on to current output stream
    template <class T>
    DebugStream& operator<<(T data)  {
      if (activator<thislevel, dlevel>::value && _active)
        current->out << data;

      return *this;
    };

    //! pass on manipulators to underlying output stream
    DebugStream& operator<<(std::ostream& (*f)(std::ostream&)) {
      if (activator<thislevel, dlevel>::value && _active)
        f(current->out);
      return *this;
    };

    //! set activation flag and store old value
    void push(bool b) {
      _actstack.push(_active);
      _active = b;
    };

    //! restore previously set activation flag
    void pop() {
      if (_actstack.empty())
        DUNE_THROW(DebugStreamError, "No previous activation setting!");

      _active = _actstack.top();
      _actstack.pop();
    };

    //! reports if this stream is active
    bool active() const {
      return _active;
    };

    //! set output to a different stream. Old stream data is stored
    void attach(std::ostream& stream) {
      // a bit of paranoia :)
      current->out.flush();

      streamwrap* newcurr = new streamwrap(stream);
      newcurr->next = current;
      current = newcurr;
    };

    //! detach current output stream and restore to previous stream
    void detach() {
      current->out.flush();

      if (current->next == 0)
        DUNE_THROW(DebugStreamError, "Cannot detach initial stream!");

      streamwrap* old = current;
      current = current->next;
      delete old;
    };

  private:
    //! flag to switch output during runtime
    bool _active;

    /*! store old activation settings so that the outside code doesn't
       need to remeber */
    std::stack<bool> _actstack;

    //! internal class so that a "stack" of ostream-references is possible
    class streamwrap {
    public:
      streamwrap(std::ostream& _out) : out(_out) {};

      std::ostream& out;
      streamwrap* next;
    };

    //! current output stream and link to possibly pushed old output streams
    streamwrap* current;
  };

};

#endif
