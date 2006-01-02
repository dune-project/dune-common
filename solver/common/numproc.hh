// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_NUMPROC_HH
#define DUNE_NUMPROC_HH


namespace Dune {

  /** \brief Exception thrown by solvers */
  class SolverError : public Exception {};

  /** \brief Base class for numerical procedures */
  class NumProc
  {
  public:

    NumProc() : verbosity_(FULL) {}

    /** \brief Different levels of verbosity */
    enum VerbosityMode {QUIET, REDUCED, FULL};

    /** \brief Controls the amount of screen output of a numproc */
    VerbosityMode verbosity_;

  };

} // namespace Dune

#endif
