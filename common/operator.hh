// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_OPERATOR_HH__
#define __DUNE_OPERATOR_HH__

#include "mapping.hh"

namespace Dune
{

  template <typename Field, typename Domain, typename Range>
  class Operator : public Mapping <Field,Domain,Range>
  {
  public:

  private:

  };

}

#endif
