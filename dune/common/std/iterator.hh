// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_ITERATOR_HH
#define DUNE_COMMON_STD_ITERATOR_HH

#include <concepts>
#include <iterator>

namespace Dune::Std {

/**
 * \brief The `std::indirectly_copyable` concept specifies the relationship between
 * an `indirectly_readable` type and a type that is `indirectly_writable`. The
 * `indirectly_writable` type must be able to directly copy the object that the
 * `indirectly_readable` type references.
 *
 * The standard implementation is available in libstdc++ >= 10 and libc++ >= 15
 */
template <class In, class Out>
concept indirectly_copyable =
    std::indirectly_readable<In> &&
    std::indirectly_writable<Out, std::iter_reference_t<In>>;

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_ITERATOR_HH
