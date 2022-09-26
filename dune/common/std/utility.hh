// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_UTILITY_HH
#define DUNE_COMMON_STD_UTILITY_HH

#include <utility>

#warning dune/common/std/utility.hh is deprecated and will be removed after Dune 2.8.\
 Include <utility> instead

namespace Dune
{

  namespace Std
  {

    using std::integer_sequence;
    using std::index_sequence;
    using std::make_integer_sequence;
    using std::make_index_sequence;
    using std::index_sequence_for;

  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_UTILITY_HH
