// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/*

   Implements a generic grid check

 */

template <class Grid>
void gridcheck (Grid &g) {
  // internal check if interface is complete
  g.checkIF();
};
