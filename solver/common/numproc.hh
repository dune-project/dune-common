// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_NUMPROC_HH__
#define __DUNE_NUMPROC_HH__


namespace Dune {

  //! \todo Please doc me!
  class NumProc
  {
  public:

    enum VerbosityMode {QUIET, REDUCED, FULL};

    VerbosityMode verbosity_;

  };

} // namespace Dune

#endif
