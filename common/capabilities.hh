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

    template<class Grid, int codim>
    struct hasEntity
    {
      static const bool v = false;
    };

    template<class Grid>
    struct isParallel
    {
      static const bool v = false;
    };

    /*
       forward
       Capabilities::Something<const Grid>
       to
       Capabilities::Something<Grid>
     */

    template<class Grid>
    struct hasLeafIterator<const Grid>
    {
      static const bool v = Dune::Capabilities::hasLeafIterator<Grid>::v;
    };

    template<class Grid, int codim>
    struct hasEntity<const Grid, codim>
    {
      static const bool v = Dune::Capabilities::hasEntity<Grid,codim>::v;
    };

    template<class Grid>
    struct isParallel<const Grid>
    {
      static const bool v = Dune::Capabilities::isParallel<Grid>::v;
    };

  }

}

#endif // __DUNE_CAPABILITIES_HH__
