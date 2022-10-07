// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <type_traits>

#include <dune/common/typeutilities.hh>

//////////////////////////////////////////////////////////////////////
//
// check disableCopyMove
//

struct Foo {

    template< class ...Args, Dune::disableCopyMove< Foo, Args ... > = 0 >
    Foo( Args&& ... )
    {}

    Foo( const Foo& ) = delete;
    Foo( Foo&& ) = delete;
};

static_assert( std::is_default_constructible< Foo >::value, "Foo is not default constructible." );
static_assert( not std::is_copy_constructible< Foo >::value, "Foo is copy constructible." );
static_assert( not std::is_move_constructible< Foo >::value, "Foo is move constructible." );

int main()
{}
