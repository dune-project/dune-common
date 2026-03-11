// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <array>
#include <list>
#include <vector>

#include <dune/common/std/iterator.hh>

int main ()
{
  using namespace Dune;

  // test indirectly_copyable concept

  using VecConstIter = std::vector<double>::const_iterator;
  using VecIter = std::vector<double>::iterator;

  using Vec2ConstIter = std::vector<bool>::const_iterator;
  using Vec2Iter = std::vector<bool>::iterator;

  static_assert(Std::indirectly_copyable<VecConstIter, VecIter>);
  static_assert(not Std::indirectly_copyable<VecIter, VecConstIter>);

  static_assert(Std::indirectly_copyable<Vec2ConstIter, VecIter>);
  static_assert(not Std::indirectly_copyable<VecIter, Vec2ConstIter>);

  static_assert(Std::indirectly_copyable<VecConstIter, VecIter>);
  static_assert(not Std::indirectly_copyable<VecIter, VecConstIter>);


  // check copyable between different iterator types

  using ArrConstIter = std::array<double,4>::const_iterator;
  using ArrIter = std::array<double,4>::iterator;

  using Arr2ConstIter = std::array<int,4>::const_iterator;
  using Arr2Iter = std::array<int,4>::iterator;

  static_assert(Std::indirectly_copyable<VecConstIter, ArrIter>);
  static_assert(Std::indirectly_copyable<VecConstIter, Arr2Iter>);
  static_assert(Std::indirectly_copyable<ArrConstIter, VecIter>);
  static_assert(Std::indirectly_copyable<Arr2ConstIter, VecIter>);


  // check copyable between different iterator categories

  using ListConstIter = std::list<double>::const_iterator;
  using ListIter = std::list<double>::iterator;

  static_assert(Std::indirectly_copyable<VecConstIter, ListIter>);
  static_assert(Std::indirectly_copyable<ListConstIter, VecIter>);
}
