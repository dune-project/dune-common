#[=======================================================================[.rst:
Message
-------

Display a message to the user.

  message([<mode>] "message to display" ...)

The optional <mode> keyword determines the type of message:

Output Modes
^^^^^^^^^^^^

``FATAL_ERROR``
  CMake Error, stop processing and generation.
``SEND_ERROR``
  CMake Error, continue processing, but skip generation.
``WARNING``
  CMake Warning, continue processing.
``AUTHOR_WARNING``
  CMake Warning (dev), continue processing.
``DEPRECATION``
  CMake Deprecation Error or Warning if variable CMAKE_ERROR_DEPRECATED or
  CMAKE_WARN_DEPRECATED is enabled, respectively, else no message.
(none) or ``NOTICE``
  Important message printed to stderr to attract user’s attention.
``STATUS``
  The main interesting messages that project users might be interested in. Ideally these
  should be concise, no more than a single line, but still informative.
``VERBOSE``
  Detailed informational messages intended for project users. These messages should
  provide additional details that won’t be of interest in most cases, but which may be
  useful to those building the project when they want deeper insight into what’s happening.
``DEBUG``
  Detailed informational messages intended for developers working on the project itself as
  opposed to users who just want to build it. These messages will not typically be of
  interest to other users building the project and will often be closely related to
  internal implementation details.
``TRACE``
  Fine-grained messages with very low-level implementation details. Messages using this
  log level would normally only be temporary and would expect to be removed before
  releasing the project, packaging up the files, etc.

Note
^^^^

This macro is a backport of the cmake 3.15 message function.

#]=======================================================================]

include_guard(GLOBAL)

macro(message MODE TEXT)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    _message(${MODE} "${TEXT}")
  else()
    # test whether mode is an old mode
    set(old_modes "STATUS" "WARNING" "AUTHOR_WARNING" "SEND_ERROR" "FATAL_ERROR" "DEPRECATION")
    list(FIND old_modes "${MODE}" is_old_mode)
    if(NOT is_old_mode EQUAL "-1")
      _message(${MODE} "${TEXT}")
    else()
      if("${MODE}" STREQUAL "NOTICE")
        _message("${TEXT}")
      elseif("${MODE}" STREQUAL "VERBOSE")
        _message(STATUS "${TEXT}")
      elseif("${MODE}" STREQUAL "DEBUG")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log "${TEXT}\n")
      elseif("${MODE}" STREQUAL "TRACE")
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log "${TEXT}\n")
      endif()
    endif()
  endif()
endmacro(message)