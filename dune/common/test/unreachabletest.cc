// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <cassert>

#include <dune/common/unreachable.hh>


enum E { e1, e2, e3 };
enum F { f1, f2, f3, f4 };

int foo(E e)
{
  int x = 0;
  switch (e)
  {
    case e1: x = 1; break;
    case e2: x = 2; break;
    case e3: x = 3; break;
    default: DUNE_UNREACHABLE;
  }

  return x;
}

int bar(F f)
{
  switch (f)
  {
    case f1: return 1;
    case f2: return 2;
    case f3: return 3;
    default: DUNE_UNREACHABLE_MSG("Unhandled enumeration value of enum 'F'");
  }
}

int main()
{
  E e = e3;
  int x = foo(e);

#ifndef NDEBUG
  F f = f4;
#else
  F f = f3;
#endif
  try {
    int y = bar(f); // should throw an exception in !NDEBUG mode, since f4 not handled correctly
    assert(false && "Exception not thrown as expected");
  } catch(Dune::Exception ex) {}
}
