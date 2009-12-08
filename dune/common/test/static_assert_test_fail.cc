// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/static_assert.hh>

int main()
{
  dune_static_assert(false, "FAIL");
  return 0;
}
