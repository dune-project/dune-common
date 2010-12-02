// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstddef>

#include <dune/common/static_assert.hh>
#include <dune/common/tuples.hh>
#include <dune/common/tupleutility.hh>

//////////////////////////////////////////////////////////////////////
//
// check FirstTypeIndex
//
typedef Dune::tuple<int, unsigned, double> MyTuple;
dune_static_assert((Dune::FirstTypeIndex<MyTuple, int>::value == 0),
                   "FirstTypeIndex finds the wrong index for double in "
                   "MyTuple!");
dune_static_assert((Dune::FirstTypeIndex<MyTuple, unsigned>::value == 1),
                   "FirstTypeIndex finds the wrong index for double in "
                   "MyTuple!");
dune_static_assert((Dune::FirstTypeIndex<MyTuple, double>::value == 2),
                   "FirstTypeIndex finds the wrong index for double in "
                   "MyTuple!");

int main() {}
