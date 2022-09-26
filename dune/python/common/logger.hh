// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_LOGGER_HH
#define DUNE_PYTHON_COMMON_LOGGER_HH

#include <string>
#include <utility>

#include <dune/common/visibility.hh>

#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace Python
  {

    // Logger
    // ------

    struct DUNE_PRIVATE Logger
    {
      enum class Level : int
      {
        critical = 50,
        error = 40,
        warning = 30,
        info = 20,
        debug = 10,
        notSet = 0
      };

      explicit Logger ( const std::string &name )
        : logging_( pybind11::module::import( "logging" ) ),
          logger_( logging_.attr( "getLogger" )( name ) )
      {}

      template< class... Args >
      void critical ( const std::string &msg, Args &&... args ) const
      {
        log( Level::critical, msg, std::forward< Args >( args )... );
      }

      template< class... Args >
      void error ( const std::string &msg, Args &&... args ) const
      {
        log( Level::error, msg, std::forward< Args >( args )... );
      }

      template< class... Args >
      void warning ( const std::string &msg, Args &&... args ) const
      {
        log( Level::warning, msg, std::forward< Args >( args )... );
      }

      template< class... Args >
      void info ( const std::string &msg, Args &&... args ) const
      {
        log( Level::info, msg, std::forward< Args >( args )... );
      }

      template< class... Args >
      void debug ( const std::string &msg, Args &&... args ) const
      {
        log( Level::debug, msg, std::forward< Args >( args )... );
      }

      template< class... Args >
      void log ( int level, const std::string &msg, Args &&... args ) const
      {
        pybind11::object pyLevel = pybind11::int_( level );
        logger_.attr( "log" )( pyLevel, msg, *pybind11::make_tuple( std::forward< Args >( args )... ) );
      }

      template< class... Args >
      void log ( Level level, const std::string &msg, Args &&... args ) const
      {
        log( static_cast< int >( level ), msg, std::forward< Args >( args )... );
      }

      void setLevel ( int level ) { logger_.attr( "setLevel" )( level ); }

      bool isEnabledFor ( int level ) { return pybind11::cast< bool >( logger_.attr( "isEnabledFor" )( level ) ); }

      int getEffectiveLevel () { return pybind11::cast< int >( logger_.attr( "getEffectiveLevel" )() ); }

    private:
      pybind11::module logging_;
      pybind11::object logger_;
    };

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_LOGGER_HH
