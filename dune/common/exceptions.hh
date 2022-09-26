// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_EXCEPTIONS_HH
#define DUNE_EXCEPTIONS_HH

#include <exception>
#include <string>
#include <sstream>

namespace Dune {

  /*! \defgroup Exceptions Exception handling
     \ingroup Common
     \{

     The Dune-exceptions are designed to allow a simple derivation of subclasses
     and to accept a text written in the '<<' syntax.

     Example of usage:

     \code
     #include <dune/common/exceptions.hh>

     ...

     class FileNotFoundError : public Dune::IOError {};

     ...

     void fileopen (std::string name) {
     std::ifstream file;

     file.open(name.c_str());

     if (file == 0)
      DUNE_THROW(FileNotFoundError, "File " << name << " not found!");

     ...

     file.close();
     }

     ...

     int main () {
     try {
      ...
     } catch (Dune::IOError &e) {
      std::cerr << "I/O error: " << e << std::endl;
      return 1;
     } catch (Dune::Exception &e) {
      std::cerr << "Generic Dune error: " << e << std::endl;
      return 2;
     }
     }
     \endcode

     \see exceptions.hh for detailed info

   */

  /*! \file
     \brief A few common exception classes

     This file defines a common framework for generating exception
     subclasses and to throw them in a simple manner

   */

  /* forward declarations */
  class Exception;
  struct ExceptionHook;

  /*! \class Exception
     \brief Base class for Dune-Exceptions

     all Dune exceptions are derived from this class via trivial subclassing:

     \code
     class MyException : public Dune::Exception {};
     \endcode

     You should not \c throw a Dune::Exception directly but use the macro
     DUNE_THROW() instead which fills the message-buffer of the exception
     in a standard way and features a way to pass the result in the
     operator<<-style

     \see DUNE_THROW, IOError, MathError

   */
  class Exception
  : public std::exception
  {
  public:
    Exception ();
    void message(const std::string &msg); //!< store string in internal message buffer
    const char* what() const noexcept override; //!< output internal message buffer
    static void registerHook (ExceptionHook * hook); //!< add a functor which is called before a Dune::Exception is emitted (see Dune::ExceptionHook) \see Dune::ExceptionHook
    static void clearHook ();                  //!< remove all hooks
  private:
    std::string _message;
    static ExceptionHook * _hook;
  };

  /*! \brief Base class to add a hook to the Dune::Exception

     The user can add a functor which should be called before a Dune::Exception is emitted.


     Example: attach a debugger to the process, if an exception is thrown
     \code
     struct ExceptionHookDebugger : public Dune::ExceptionHook
     {
      char * process_;
      char * debugger_;
      ExceptionHookDebugger (int argc, char ** argv, std::string debugger)
      {
          process_ = strdup(argv[0]);
          debugger_ = strdup(debugger.c_str());
      }
      virtual void operator () ()
      {
          pid_t pid = getpid();
          pid_t cpid;
          cpid = fork();
          if (cpid == 0) // child
          {
              char * argv[4];
              argv[0] = debugger_;
              argv[1] = process_;
              argv[2] = new char[12];
              snprintf(argv[2], 12, "%i", int(pid));
              argv[3] = 0;
              // execute debugger
              std::cout << process_ << "\n";
              std::cout << argv[0] << " "
                        << argv[1] << " "
                        << argv[2] << std::endl;
              execv(argv[0], argv);
          }
          else // parent
          {
              // send application to sleep
              kill(pid, SIGSTOP);
          }
      }
     };
     \endcode

     This hook is registered via a static method of Dune::Exception:
     \code
     int main(int argc, char** argv) {
      Dune::MPIHelper & mpihelper = Dune::MPIHelper::instance(argc,argv);
      ExceptionHookDebugger debugger(argc, argv, "/usr/bin/ddd");
      Dune::Exception::registerHook(& debugger);
      try
      {
          ...
      }
      catch (std::string & s) {
          std::cout << mpihelper.rank() << ": ERROR: " << s << std::endl;
      }
      catch (Dune::Exception & e) {
          std::cout << mpihelper.rank() << ": DUNE ERROR: " << e.what() << std::endl;
      }
     }
     \endcode

   */
  struct ExceptionHook
  {
    virtual ~ExceptionHook() {}
    virtual void operator () () = 0;
  };

  inline std::ostream& operator<<(std::ostream &stream, const Exception &e)
  {
    return stream << e.what();
  }

#ifndef DOXYGEN
  // the "format" the exception-type gets printed.  __FILE__ and
  // __LINE__ are standard C-defines, the GNU cpp-infofile claims that
  // C99 defines __func__ as well. __FUNCTION__ is a GNU-extension
#define THROWSPEC(E) # E << " [" << __func__ << ":" << __FILE__ << ":" << __LINE__ << "]: "
#endif // DOXYGEN

  /*! Macro to throw an exception

     \code
     #include <dune/common/exceptions.hh>
     \endcode

     \param E exception class derived from Dune::Exception
     \param m reason for this exception in ostream-notation

     Example:

     \code
     if (filehandle == 0)
      DUNE_THROW(FileError, "Could not open " << filename << " for reading!");
     \endcode

     DUNE_THROW automatically adds information about the exception thrown
     to the text.

     \note
     you can add a hook to be called before a Dune::Exception is emitted,
     e.g. to add additional information to the exception,
     or to invoke a debugger during parallel debugging. (see Dune::ExceptionHook)

   */
  // this is the magic: use the usual do { ... } while (0) trick, create
  // the full message via a string stream and throw the created object
#define DUNE_THROW(E, m) do { E th__ex; std::ostringstream th__out; \
                              th__out << THROWSPEC(E) << m; th__ex.message(th__out.str()); throw th__ex; \
} while (0)

  /*! \brief Default exception class for I/O errors

     This is a superclass for any errors dealing with file/socket I/O problems
     like

     - file not found
     - could not write file
     - could not connect to remote socket
   */
  class IOError : public Exception {};

  /*! \brief Default exception class for mathematical errors

     This is the superclass for all errors which are caused by
     mathematical problems like

     - matrix not invertible
     - not convergent
   */
  class MathError : public Exception {};

  /*! \brief Default exception class for range errors

     This is the superclass for all errors which are caused because
     the user tries to access data that was not allocated before.
     These can be problems like

     - accessing array entries behind the last entry
     - adding the fourth non zero entry in a sparse matrix
       with only three non zero entries per row

   */
  class RangeError : public Exception {};

  /*! \brief Default exception for dummy implementations

     This exception can be used for functions/methods

     - that have to be implemented but should never be called
     - that are missing
   */
  class NotImplemented : public Exception {};

  /*! \brief Default exception class for OS errors

     This class is thrown when a system-call is used and returns an
     error.

   */
  class SystemError : public Exception {};

  /*! \brief Default exception if memory allocation fails

   */
  class OutOfMemoryError : public SystemError {};

  /*! \brief Default exception if a function was called while
     the object is not in a valid state for that function.
   */
  class InvalidStateException : public Exception {};

  /*! \brief Default exception if an error in the parallel
     communication of the program occurred
     \ingroup ParallelCommunication
   */
  class ParallelError : public Exception {};

} // end namespace

#endif
