// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/typeutility.hh>

//////////////////////////////////////////////////////////////////////
//
// check disableCopyMove
//

struct Foo {

    template< class ...Args, disableCopyMove< Foo, Args ... > = 0 >
    Foo( Args&& ... )
    {}

    Foo( const Foo& ) = delete;
    Foo( Foo&& ) = delete;
};

static_assert( not std::is_copy_constructible< Foo >::value, "Foo is copy constructible." );
static_assert( not std::is_move_constructible< Foo >::value, "Foo is move constructible." );
static_assert( not std::is_default_constructible< Foo >::value, "Foo is default constructible." );

int main()
{}
