// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_GETDIMENSION_HH
#define DUNE_PYTHON_COMMON_GETDIMENSION_HH

#include <type_traits>

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

template< class T, class=void >
struct GetDimension;

template< class T >
struct GetDimension< T, std::enable_if_t<std::is_arithmetic<T>::value>>
   : public std::integral_constant< int, 1 > {};
template< class FT, int dim >
struct GetDimension<Dune::FieldVector<FT,dim>>
   : public std::integral_constant< int, dim > {};
template< class FT, int dimr, int dimc >
struct GetDimension<Dune::FieldMatrix<FT,dimr,dimc>>
   : public std::integral_constant< int, dimr*dimc > {};

#endif // DUNE_PYTHON_COMMON_GETDIMENSION_HH
