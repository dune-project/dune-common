// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INTERFACES_HH
#define DUNE_INTERFACES_HH

namespace Dune {

  //! An interface class for cloneable objects
  struct Cloneable {
    virtual Cloneable* clone() const = 0;
  };

} // end namespace Dune

#endif
