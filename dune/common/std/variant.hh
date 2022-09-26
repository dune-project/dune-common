// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_VARIANT_HH
#define DUNE_COMMON_STD_VARIANT_HH

#warning dune/common/std/variant.hh is deprecated and will be removed after Dune 2.8.\
 Include <variant> instead

#include <variant>

namespace Dune {
namespace Std {
  using std::variant;
  using std::visit;
  using std::variant_size;
  using std::variant_size_v;
  using std::get;
  using std::get_if;
  using std::holds_alternative;
  using std::monostate;
}
}

#endif
