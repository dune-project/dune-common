// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef STUBS_H
#define STUBS_H

/* Forward declaration of the loopstubs */
namespace Dune {
  namespace PMGStubs {
    enum typ { Inner, Border };

    template <class GRID, int SMOOTHER, int TYP> class GaussSeidel;
    template <class GRID, int SMOOTHER, int TYP> class Defect;
    template <class GRID, int SMOOTHER, int TYP> class Restrict;
    template <class GRID, int SMOOTHER, int TYP> class Prolongate;
    template <class GRID, int SMOOTHER> class InitExchange;
    template <class GRID> class InitIterator;
  }
}

#endif // STUBS_H
