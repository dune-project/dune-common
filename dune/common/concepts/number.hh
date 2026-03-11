// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONCEPTS_NUMBER_HH
#define DUNE_COMMON_CONCEPTS_NUMBER_HH

#include <dune/common/typetraits.hh>

namespace Dune::Concept {

/**
 * \brief The `Number` concept is satisfied if the type `N` can act as a scalar
 * in the context of (hierarchically blocked) containers.
 *
 * Number types are possible element types of (hierarchically blocked) matrices
 * and vectors, such as `FieldMatrix`, `FieldVector`, `BCRSMatrix`, `BlockVector`,
 * `MultiTypeBlockVector`, etc. It allows, for example, to define break conditions
 * in recursive algorithms and are used as value initializers and scalar factors
 * in scaling operations.
 *
 * Types that can act as number types include the arithmetic types like `double`,
 * `int`, or `float`, and the `std::complex<U>` types if `U` itself is a scalar
 * type. It is possible to register used-defined types, e.g., extended precision
 * types or automatic differentiation types, or anything else that might sensibly
 * be an element of a matrix or vector. Therefore, the traits class `Dune::IsNumber`
 * can be specialized.
 *
 * \note The precise semantics of number types are only vaguely specified. It
 * should behave similar to arithmetic types, e.g., provide arithmetic operations,
 * and should be considered a field type in the mathematical sense.
 */
template <class N>
concept Number = Dune::IsNumber<N>::value;

} // end namespace Dune::Concept

#endif // DUNE_COMMON_CONCEPTS_NUMBER_HH
