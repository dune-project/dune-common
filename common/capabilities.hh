// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_CAPABILITIES_HH__
#define __DUNE_CAPABILITIES_HH__

namespace Dune
{

  namespace Capabilities
  {

    template<class Grid>
    struct hasLeafIterator
    {
      static const bool v = false;
    };

    template<class Grid, class Entity>
    struct hasEntity
    {
      static const bool v = false;
    };

    template<class Grid>
    struct isParallel
    {
      static const bool v = false;
    };

  }

}

#endif // __DUNE_CAPABILITIES_HH__
