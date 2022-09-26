// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_STRING_HH
#define DUNE_PYTHON_COMMON_STRING_HH

#include <string>
#include <type_traits>

namespace Dune
{

  namespace Python
  {

    using std::to_string;



    // join
    // ----

    template< class Formatter, class Iterator >
    inline static auto join ( const std::string &delimiter, Formatter &&formatter, Iterator begin, Iterator end )
      -> std::enable_if_t< std::is_same< std::decay_t< decltype( formatter( *begin ) ) >, std::string >::value, std::string >
    {
      std::string s;
      if( begin != end )
      {
        for( s = formatter( *begin++ ); begin != end; s += formatter( *begin++ ) )
          s += delimiter;
      }
      return s;
    }

    template< class Iterator >
    inline static auto join ( const std::string &delimiter, Iterator begin, Iterator end )
      -> std::enable_if_t< std::is_same< std::decay_t< decltype( *begin ) >, std::string >::value, std::string >
    {
      return join( delimiter, [] ( decltype( *begin ) s ) -> decltype( *begin ) { return s; }, begin, end );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_STRING_HH
